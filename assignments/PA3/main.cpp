#include <iostream>
#include <fstream>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "PrintVisitor.h"

std::stringstream GetExpected(const std::string& lexer_path, const std::string& parser_path, const std::string& file_name) {
    std::system((lexer_path + " " + file_name + " | " + parser_path + " > expected.txt 2>&1").c_str());
    std::ifstream expected_file("expected.txt");
    std::stringstream expected_result;
    expected_result << expected_file.rdbuf();
    expected_file.close();
    std::remove("expected.txt");

    return expected_result;
}

std::stringstream GetActual(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("File " + file_name + " wasn't found");
    }

    std::vector<Token> tokens;
    auto lexer = lexer::Lexer(file, file_name);
    while (lexer.HasNext()) {
        tokens.push_back(lexer.Next());
    }
    file.close();

    std::stringstream actual_result;
    try {
        parser::Program program = parser::Parser(tokens).ParseProgram();
        auto printer = parser::PrintVisitor();
        printer.VisitProgram(&program);
        actual_result << printer.GetResult() << std::endl;
    } catch (std::runtime_error &e) {
        actual_result << e.what() << std::endl;
    }
    return actual_result;
}

int main(int argc, char** argv) {
    auto lexer_path = std::string(argv[1]);
    auto parser_path = std::string(argv[2]);
    auto file_name = std::string(argv[3]);

    auto expected_result = GetExpected(lexer_path, parser_path, file_name);
    auto actual_result = GetActual(file_name);

    std::string actual;
    std::string expect;
    while (std::getline(expected_result, expect, '\n') && std::getline(actual_result, actual, '\n')) {
        if (actual == expect) {
            std::cout << actual << std::endl;
        } else {
            std::cerr << "Lines are not equal." << std::endl;
            std::cerr << "Expected:" << std::endl << expect << std::endl;
            std::cerr << "Actual:" << std::endl << actual << std::endl;
            return 1;
        }
    }
    std::cout << actual.c_str() << std::endl;
    std::cout << expect.c_str() << std::endl;
    return 0;
}