#include <iostream>
#include <fstream>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "PrintVisitor.h"
#include "SemanticAnalyzer.h"
#include "CoolBackend.h"

inline bool EndsWith(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline std::string DropExtension(std::string const & value) {
    return value.substr(0, value.size() - 3);
}

std::stringstream RunSpim(const std::string& spim_path, const std::string& file_name) {
    std::system((spim_path + " " + file_name + " > spim_result.txt").c_str());
    std::ifstream spim_file("spim_result.txt");
    std::stringstream result;
    result << spim_file.rdbuf();
    spim_file.close();
    std::remove("spim_result.txt");

    return result;
}

std::stringstream GetExpected(const std::string& coolc_path, const std::string& spim_path, const std::string& file_name) {
    std::system((coolc_path + " " + file_name).c_str());
    return RunSpim(spim_path, DropExtension(file_name) + ".s");
}

std::stringstream GetActual(const std::string& spim_path, const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("File " + file_name + " wasn't found");
    }

    std::vector<Token> tokens;
    auto lexer = lexer::SingleFileLexer({file, file_name});
    while (lexer.HasNext()) {
        tokens.push_back(lexer.Next());
    }
    file.close();

    parser::Program program = parser::Parser(tokens).ParseProgram();
    semant::SemanticAnalyzer analyzer;
    analyzer.Analyze(&program);

    std::ofstream spim_file(DropExtension(file_name) + ".s");
    spim_file << backend::CoolBackend::Convert(analyzer.GetTypeEnvironment());
    spim_file.close();

    return RunSpim(spim_path, DropExtension(file_name) + ".s");
}

int main(int argc, char** argv) {
    auto coolc_path = std::string(argv[1]);
    auto spim_path = std::string(argv[2]);
    auto file_name = std::string(argv[3]);

    auto expected_result = GetExpected(coolc_path, spim_path, file_name);
    auto actual_result = GetActual(spim_path, file_name);

    std::string actual;
    std::string expect;
    while (std::getline(expected_result, expect, '\n') && std::getline(actual_result, actual, '\n')) {
        if (expect == actual) {
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