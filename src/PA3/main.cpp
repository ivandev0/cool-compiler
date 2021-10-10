#include <iostream>
#include <fstream>
#include <filesystem>

#include "Lexer.h"
#include "Parser.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc == 1) {
        cerr << "Usage: cool_parser [file.cl]" << endl;
        return 1;
    }

    auto fileName = argv[1];
    ifstream file(fileName);

    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::vector<Token> tokens;
        auto lexer = lexer::Lexer(buffer.str());
        while (lexer.hasNext()) {
            tokens.push_back(lexer.next());
        }
        cout << parser::Parser(tokens).parseProgram().toString() << endl;
    } else {
        cerr << "File " << fileName << " wasn't found" << endl;
        return 1;
    }
    return 0;
}