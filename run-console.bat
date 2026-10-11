@echo off
cd /d "%~dp0"
if not exist build mkdir build
g++ -std=c++11 -Wall -Wextra -pedantic backend\console.cpp -o build\console.exe
if errorlevel 1 (
    pause
    exit /b 1
)
build\console.exe
pause
