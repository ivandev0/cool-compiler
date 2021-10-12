cmake_minimum_required(VERSION 3.16)

set(PARSER_HEADER_FILES
        ${cool_compiler_SOURCE_DIR}/include/PA2
        ${cool_compiler_SOURCE_DIR}/include/PA3
)

set(PARSER_CPP_FILES
        ${cool_compiler_SOURCE_DIR}/src/PA2/Token.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA2/Lexer.cpp
        ${cool_compiler_SOURCE_DIR}/assignments/PA3/main.cpp
        ${cool_compiler_SOURCE_DIR}/src/PA3
        ${cool_compiler_SOURCE_DIR}/src/PA3/Parser.cpp
)

file(GLOB tests "tests/*.test")

add_executable(parser_test ${PARSER_CPP_FILES})
target_include_directories(parser_test PRIVATE ${PARSER_HEADER_FILES})

foreach(filename ${tests})
    get_filename_component(name ${filename} NAME_WE)
    add_test("test_${name}" parser_test ${cool_compiler_SOURCE_DIR}/bin/lexer ${cool_compiler_SOURCE_DIR}/bin/parser ${filename})
endforeach()
