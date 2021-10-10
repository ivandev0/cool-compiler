cmake_minimum_required(VERSION 3.16)

set(LEXER_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2
)
set(LEXER_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2
        ${cool_compiler_SOURCE_DIR}/src/PA2/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
)

add_executable(cool_lexer ${LEXER_CPP_FILES})
target_include_directories(cool_lexer PRIVATE ${LEXER_HEADER_FILES})
