#include <iostream>
#include <fstream>
#include <filesystem>

#include "Lexer.h"
#include "Parser.h"
#include "PrintVisitor.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc == 1) {
        cerr << "Usage: cool_parser [file.cl]" << endl;
        return 1;
    }

    auto file_name = argv[1];
    ifstream file(file_name);

    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::vector<Token> tokens;
        auto lexer = lexer::Lexer(buffer.str());
        while (lexer.HasNext()) {
            tokens.push_back(lexer.Next());
        }
        parser::Program program = parser::Parser(tokens, file_name).ParseProgram();
        auto printer = parser::PrintVisitor();
        printer.VisitProgram(program);
        cout << printer.GetResult() << endl;
    } else {
        cerr << "File " << file_name << " wasn't found" << endl;
        return 1;
    }
    return 0;
}