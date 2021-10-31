cmake_minimum_required(VERSION 3.16)

set(SEMANT_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2
        ${cool_compiler_SOURCE_DIR}/include/PA3
        ${cool_compiler_SOURCE_DIR}/include/PA4
)
set(SEMANT_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA3
        ${cool_compiler_SOURCE_DIR}/src/PA3/Parser.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA4
        ${cool_compiler_SOURCE_DIR}/src/PA4/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA4/SemanticAnalyzer.cpp
)

add_executable(cool_semant ${SEMANT_CPP_FILES})
target_include_directories(cool_semant PRIVATE ${SEMANT_HEADER_FILES})
