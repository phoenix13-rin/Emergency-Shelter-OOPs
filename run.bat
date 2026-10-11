@echo off
cd /d "%~dp0"
if not exist build mkdir build
where g++ >nul 2>nul
if errorlevel 1 (
    echo Compiler not found. See README.md.
    pause
    exit /b 1
)
g++ -std=c++11 -Wall -Wextra -pedantic backend\main.cpp -o build\shelter.exe -lws2_32
if errorlevel 1 (
    pause
    exit /b 1
)
echo Open http://localhost:8080 in your browser after the server starts.
build\shelter.exe
pause
