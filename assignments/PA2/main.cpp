#include "Lexer.h"
#include <iostream>
#include <fstream>

std::stringstream GetExpected(const std::string& lexer_path, const std::string& file_name) {
    std::system((lexer_path + " " + file_name + " > expected.txt").c_str());
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

    std::stringstream actual_result;
    auto lexer = lexer::SingleFileLexer({file, file_name});
    while (lexer.HasNext()) {
        actual_result << lexer.Next().ToString() << std::endl;
    }
    file.close();
    return actual_result;
}

int main(int argc, char** argv) {
    auto lexer_path = std::string(argv[1]);
    auto file_name = std::string(argv[2]);

    auto expected_result = GetExpected(lexer_path, file_name);
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
    return 0;
}