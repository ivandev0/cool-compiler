cmake_minimum_required(VERSION 3.20)
project(cool_compiler)

set(CMAKE_CXX_STANDARD 17)

set(HEADER_FILES include/PA2/Lexer.h include/PA2/Token.h include/PA2/Utils.h)
add_executable(cool_lexer assignments/PA2/main.cpp ${HEADER_FILES} src/PA2/Lexer.cpp src/PA2/Token.cpp)
target_include_directories(cool_lexer PRIVATE include/PA2)
