#!/bin/sh
mkdir -p "cmake-build-debug"
mkdir -p "cmake-build-debug_asan"
mkdir -p "cmake-build-debug_ubsan"

cmake --build cmake-build-debug/
cmake -DCMAKE_BUILD_TYPE=ASAN --build cmake-build-debug_asan/
cmake -DCMAKE_BUILD_TYPE=UBSAN --build cmake-build-debug_ubsan/