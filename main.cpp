#include "web.hpp"

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        if (argc > 1) {
            std::size_t used = 0;
            port = std::stoi(argv[1], &used);
            if (used != std::string(argv[1]).size() || port < 1024 || port > 65535) {
                throw ShelterException("Port must be between 1024 and 65535.");
            }
        }

        Shelter shelter(50);       // Create one shelter with 50 spaces.
        shelter.reset();           // Add small example data.
        web::HttpServer server(shelter, "frontend", port);
        server.run();              // Keep listening for browser requests.
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
