cmake_minimum_required(VERSION 3.20)

set(LEXER_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2/Lexer.h
        ${cool_compiler_SOURCE_DIR}/include/PA2/Token.h
        ${cool_compiler_SOURCE_DIR}/include/PA2/Utils.h
)
set(LEXER_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/assignments/PA2/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
)

file(GLOB tests "tests/*.cool")

add_executable(lexer_test ${LEXER_CPP_FILES})
target_include_directories(lexer_test PRIVATE ${cool_compiler_SOURCE_DIR}/include/PA2)

foreach(filename ${tests})
    get_filename_component(name ${filename} NAME_WE)
    add_test("test_${name}" lexer_test ${cool_compiler_SOURCE_DIR}/bin/lexer ${filename})
endforeach()
