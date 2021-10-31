#!/bin/sh
cd cmake-build-debug/
ctest
cd ..

cd cmake-build-debug_asan/
ctest
cd ..

cd cmake-build-debug_ubsan/
ctest
cd ..