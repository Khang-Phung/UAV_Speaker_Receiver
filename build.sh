#!/bin/bash

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "=================================="
echo " UAV Speaker Receiver Build"
echo "=================================="

# Check dependencies
for cmd in cmake ninja g++; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "ERROR: $cmd not found!"
        exit 1
    fi
done

echo "[1/2] Configuring..."

cmake -S "$PROJECT_DIR" \
      -B "$BUILD_DIR" \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Release

echo "[2/2] Compiling..."

cmake --build "$BUILD_DIR" --parallel "$(nproc)"

echo "=================================="
echo " BUILD SUCCESS"
echo " Output: $BUILD_DIR/receiver"
echo "=================================="