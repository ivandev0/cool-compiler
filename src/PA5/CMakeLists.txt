cmake_minimum_required(VERSION 3.16)

set(SEMANT_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2
        ${cool_compiler_SOURCE_DIR}/include/PA3
        ${cool_compiler_SOURCE_DIR}/include/PA4
        ${cool_compiler_SOURCE_DIR}/include/PA5
)
set(SEMANT_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA3
        ${cool_compiler_SOURCE_DIR}/src/PA3/Parser.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA4
        ${cool_compiler_SOURCE_DIR}/src/PA4/SemanticAnalyzer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA5
        ${cool_compiler_SOURCE_DIR}/src/PA5/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA5/CoolBackend.cpp
)

add_executable(cool_bin ${SEMANT_CPP_FILES})
target_include_directories(cool_bin PRIVATE ${SEMANT_HEADER_FILES})
