cmake_minimum_required(VERSION 3.16)

set(LEXER_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2/Lexer.h
        ${cool_compiler_SOURCE_DIR}/include/PA2/Token.h
        ${cool_compiler_SOURCE_DIR}/include/PA2/Utils.h
)
set(LEXER_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
)

add_executable(cool_lexer ${LEXER_CPP_FILES})
target_include_directories(cool_lexer PRIVATE ${cool_compiler_SOURCE_DIR}/include/PA2)
