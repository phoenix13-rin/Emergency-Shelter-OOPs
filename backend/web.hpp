// Networking adapter. Study resource.hpp and shelter.hpp first.
// This file translates HTTP requests into Shelter method calls.
#pragma once
#include "json.hpp"
#include <algorithm>
#include <fstream>
#include <array>
#include <cctype>
#include <iostream>
#include <map>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using Socket=SOCKET;
inline void closeSocket(Socket s){
    closesocket(s);
}
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
using Socket=int;
constexpr Socket INVALID_SOCKET=-1;
inline void closeSocket(Socket s){
    close(s);
}
#endif
namespace web {
    class Connection {
        Socket socket_;
        public: explicit Connection(Socket s):socket_(s){
        } ~Connection(){
            closeSocket(socket_);
        } Connection(const Connection&)=delete;
        Connection& operator=(const Connection&)=delete;
        Socket get()const{
            return socket_;
        }
    };
    // RAII
    inline std::string decode(const std::string& s){
        std::string out;
        for(size_t i=0;i<s.size();i++){
            if(s[i]=='+')out+=' ';
            else if(s[i]=='%'){
                if(i+2>=s.size())throw ShelterException("Malformed form data.");
                auto hex=[](char c){
                    if(c>='0'&&c<='9')return c-'0';
                    if(c>='a'&&c<='f')return c-'a'+10;
                    if(c>='A'&&c<='F')return c-'A'+10;
                    throw ShelterException("Malformed URL encoding.");
                };
                out+=char(hex(s[i+1])*16+hex(s[i+2]));
                i+=2;
            }else out+=s[i];
        }return out;
    }
    inline std::map<std::string,std::string> parseForm(const std::string& s){
        std::map<std::string,std::string> result;
        std::istringstream in(s);
        std::string pair;
        while(std::getline(in,pair,'&')){
            auto p=pair.find('=');
            if(p==std::string::npos)throw ShelterException("Invalid form data.");
            result[decode(pair.substr(0,p))]=decode(pair.substr(p+1));
        }return result;
    }
    inline int number(const std::map<std::string,std::string>& f,const std::string& key){
        auto it=f.find(key);
        if(it==f.end())throw ShelterException("Missing field: "+key);
        size_t used=0;
        int n;
        try{
            n=std::stoi(it->second,&used);
        }catch(...){
            throw ShelterException("Invalid number: "+key);
        }if(used!=it->second.size())throw ShelterException("Invalid number: "+key);
        return n;
    }
    inline std::string field(const std::map<std::string,std::string>& f,const std::string& key){
        auto it=f.find(key);
        if(it==f.end())throw ShelterException("Missing field: "+key);
        return it->second;
    }
    inline void respond(Socket socket,int status,const std::string& type,const std::string& body){
        std::string reason=status==200?"OK":status==400?"Bad Request":status==404?"Not Found":status==403?"Forbidden":"Internal Server Error";
        std::string data="HTTP/1.1 "+std::to_string(status)+" "+reason+"\r\nContent-Type: "+type+"\r\nContent-Length: "+std::to_string(body.size())+"\r\nConnection: close\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n\r\n"+body;
        size_t offset=0;
        while(offset<data.size()){
#ifdef _WIN32
            int n=send(socket,data.data()+offset,int(data.size()-offset),0);
#else
            #ifdef MSG_NOSIGNAL
int n=send(socket,data.data()+offset,data.size()-offset,MSG_NOSIGNAL);
#else
int n=send(socket,data.data()+offset,data.size()-offset,0);
#endif
#endif
            if(n<=0)return;
            offset+=size_t(n);
        }
    }
    class HttpServer {
        Shelter& shelter_;
        std::string frontend_;
        int port_;
        void handle(Socket client){
            try {
                std::string request;
                std::array<char,4096> buffer{
                };
                size_t split;
                while((split=request.find("\r\n\r\n"))==std::string::npos){
                    int n=recv(client,buffer.data(),int(buffer.size()),0);
                    if(n<=0)return;
                    request.append(buffer.data(),n);
                    if(request.size()>32768)throw ShelterException("Request too large.");
                }
                std::istringstream headers(request.substr(0,split));
                std::string method,path,version;
                headers>>method>>path>>version;
                std::string line,host,origin;
                size_t length=0;
                std::getline(headers,line);
                while(std::getline(headers,line)){
                    if(!line.empty()&&line.back()=='\r')line.pop_back();
                    auto colon=line.find(':');
                    if(colon==std::string::npos)continue;
                    auto key=line.substr(0,colon),value=line.substr(colon+1);
                    std::transform(key.begin(),key.end(),key.begin(),[](unsigned char c){
                        return char(std::tolower(c));
                    });
                    while(!value.empty()&&value.front()==' ')value.erase(value.begin());
                    if(key=="content-length"){
                        size_t used=0;
                        try{
                            length=std::stoul(value,&used);
                        }catch(...){
                            throw ShelterException("Invalid request length.");
                        }if(used!=value.size()||length>16384)throw ShelterException("Request too large.");
                    }if(key=="host")host=value;
                    if(key=="origin")origin=value;
                    if(key=="transfer-encoding")throw ShelterException("Chunked requests are unsupported.");
                }
                const auto localhost="localhost:"+std::to_string(port_),loopback="127.0.0.1:"+std::to_string(port_);
                if(host!=localhost&&host!=loopback){
                    respond(client,403,"application/json","{\"error\":\"Invalid host.\"}");
                    return;
                }
                if(method=="POST" && origin!="http://"+localhost && origin!="http://"+loopback){
                    respond(client,403,"application/json","{\"error\":\"Use the frontend served by this app.\"}");
                    return;
                }
                while(request.size()-split-4<length){
                    int n=recv(client,buffer.data(),int(buffer.size()),0);
                    if(n<=0)throw ShelterException("Incomplete request.");
                    request.append(buffer.data(),n);
                }
                if(method=="GET"&&path=="/api/state"){
                    respond(client,200,"application/json; charset=utf-8",serialize(shelter_));
                    return;
                }
                if(method=="POST") {
                    auto form=parseForm(request.substr(split+4,length));
                    // Browser actions call the same simple Shelter methods as the console.
                    if(path=="/api/resources") {
                        std::string type = field(form,"type");
                        if(type=="Water") shelter_.addWater(field(form,"name"),number(form,"quantity"));
                        else if(type=="Food") shelter_.addFood(field(form,"name"),number(form,"quantity"),field(form,"expiry"));
                        else throw ShelterException("Choose Water or Food.");
                    }
                    else if(path=="/api/residents") shelter_.admit(field(form,"name"),number(form,"members"));
                    else if(path=="/api/allocations") shelter_.allocate(number(form,"residentId"),number(form,"resourceId"),number(form,"quantity"));
                    else if(path=="/api/restock") shelter_.restock(number(form,"resourceId"),number(form,"quantity"),field(form,"donor"));
                    else if(path=="/api/reset")shelter_.reset();
                    else {
                        respond(client,404,"application/json","{\"error\":\"Endpoint not found.\"}");
                        return;
                    }
                    respond(client,200,"application/json; charset=utf-8",serialize(shelter_));
                    return;
                }
                if(method=="GET") {
                    const std::map<std::string,std::pair<std::string,std::string>> files={
                        {
                            "/",{
                                "index.html","text/html"
                            }
                        },{
                            "/index.html",{
                                "index.html","text/html"
                            }
                        },{
                            "/style.css",{
                                "style.css","text/css"
                            }
                        },{
                            "/app.js",{
                                "app.js","text/javascript"
                            }
                        }
                    };
                    auto it=files.find(path);
                    if(it!=files.end()){
                        std::ifstream in(frontend_+"/"+it->second.first,std::ios::binary);
                        if(!in)throw ShelterException("Frontend files missing. Start from the project folder.");
                        std::ostringstream out;
                        out<<in.rdbuf();
                        respond(client,200,it->second.second+"; charset=utf-8",out.str());
                        return;
                    }
                }
                respond(client,404,"application/json","{\"error\":\"Not found.\"}");
            }catch(const ShelterException& e){
                respond(client,400,"application/json; charset=utf-8","{\"error\":"+jsonString(e.what())+"}");
            }
            catch(const std::exception& e){
                std::cerr<<"Server error: "<<e.what()<<'\n';
                respond(client,500,"application/json","{\"error\":\"Unable to complete operation. See server terminal.\"}");
            }
        }
        public:
        HttpServer(Shelter& shelter,const std::string& frontend,int port):shelter_(shelter),frontend_(frontend),port_(port){
        }
        void run(){
#ifdef _WIN32
            WSADATA data;
            if(WSAStartup(MAKEWORD(2,2),&data)!=0)throw ShelterException("Could not initialize networking.");
#endif
            Socket raw=socket(AF_INET,SOCK_STREAM,0);
            if(raw==INVALID_SOCKET)throw ShelterException("Could not create socket.");
            Connection server(raw);
            int reuse=1;
            setsockopt(raw,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&reuse),sizeof(reuse));
            sockaddr_in addr{
            };
            addr.sin_family=AF_INET;
            addr.sin_port=htons(static_cast<unsigned short>(port_));
            addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
            if(bind(raw,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))!=0||listen(raw,16)!=0)throw ShelterException("Cannot listen on port. Close the other server or select another port.");
            std::cout<<"Simple Shelter running at http://localhost:"<<port_<<"\nPress Ctrl+C to stop.\n"<<std::flush;
            while(true){
                Socket client=accept(raw,nullptr,nullptr);
                if(client==INVALID_SOCKET)continue;
                Connection connection(client);
#ifdef _WIN32
                DWORD timeout=3000;
                setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
                setsockopt(client,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
#else
                
#ifdef SO_NOSIGPIPE
int noPipe=1;setsockopt(client,SOL_SOCKET,SO_NOSIGPIPE,&noPipe,sizeof(noPipe));
#endif
timeval timeout{
                    3,0
                };
                setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
                setsockopt(client,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
#endif
                handle(client);
            }
        }
    };
}
