cmake_minimum_required(VERSION 3.16)

set(PARSER_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2
        ${cool_compiler_SOURCE_DIR}/include/PA3
)
set(PARSER_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA3
        ${cool_compiler_SOURCE_DIR}/src/PA3/Parser.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA3/main.cpp
)

add_executable(cool_parser ${PARSER_CPP_FILES})
target_include_directories(cool_parser PRIVATE ${PARSER_HEADER_FILES})
