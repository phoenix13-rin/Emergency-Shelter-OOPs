#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic backend/main.cpp -o build/shelter
exec ./build/shelter "${1:-8080}"
