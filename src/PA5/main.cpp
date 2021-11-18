#include <iostream>
#include <fstream>
#include <filesystem>

#include "Lexer.h"
#include "Parser.h"
#include "PrintVisitor.h"
#include "SemanticAnalyzer.h"
#include "CoolBackend.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc == 1) {
        cerr << "Usage: cool_parser [file.cl]" << endl;
        return 1;
    }

    auto file_name = argv[1];
    ifstream file(file_name);

    if (file.is_open()) {
        std::vector<Token> tokens;
        auto lexer = lexer::SingleFileLexer({file, file_name});
        while (lexer.HasNext()) {
            tokens.push_back(lexer.Next());
        }
        file.close();

        parser::Program program = parser::Parser(tokens).ParseProgram();
        semant::SemanticAnalyzer analyzer;
        analyzer.Analyze(&program);

        std::stringstream spim_program;
        backend::CoolBackend converter(spim_program);
        converter.Convert(program);

        cout << spim_program.str() << endl;
    } else {
        cerr << "File " << file_name << " wasn't found" << endl;
        return 1;
    }
    return 0;
}