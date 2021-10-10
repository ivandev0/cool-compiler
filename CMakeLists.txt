cmake_minimum_required(VERSION 3.16)
project(cool_compiler)

set(CMAKE_CONFIGURATION_TYPES "Asan;Ubsan" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_CXX_FLAGS_UBSAN "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=undefined")

enable_testing()
add_subdirectory(assignments/PA2)
add_subdirectory(src/PA2)
