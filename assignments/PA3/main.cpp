#include <iostream>
#include <fstream>

#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "PrintVisitor.h"

using namespace std;

std::stringstream getExpected(const std::string& lexerPath, const std::string& parserPath, const std::string& fileName) {
    std::system((lexerPath + " " + fileName + " | " + parserPath + " > expected.txt 2>&1").c_str());
    ifstream expectedFile("expected.txt");
    stringstream expectedResult;
    expectedResult << expectedFile.rdbuf();
    expectedFile.close();
    std::remove("expected.txt");

    return expectedResult;
}

std::stringstream getActual(const std::string& fileName) {
    ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("File " + fileName + " wasn't found");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::vector<Token> tokens;
    auto lexer = lexer::Lexer(buffer.str());
    while (lexer.hasNext()) {
        tokens.push_back(lexer.next());
    }

    stringstream actualResult;
    parser::Program program = parser::Parser(tokens, fileName).parseProgram();
    auto printer = parser::PrintVisitor();
    printer.visitProgram(program);
    actualResult << printer.getResult() << endl;
    return actualResult;
}

int main(int argc, char** argv) {
    auto lexerPath = std::string(argv[1]);
    auto parserPath = std::string(argv[2]);
    auto fileName = std::string(argv[3]);

    auto expectedResult = getExpected(lexerPath, parserPath, fileName);
    auto actualResult = getActual(fileName);

    std::string actual;
    std::string expect;
    while (std::getline(expectedResult, expect, '\n') && std::getline(actualResult, actual, '\n')) {
        if (actual == expect) {
            cout << actual << endl;
        } else {
            cerr << "Lines are not equal." << endl;
            cerr << "Expected:" << endl << expect << endl;
            cerr << "Actual:" << endl << actual << endl;
            return 1;
        }
    }
    cout << actual.c_str() << endl;
    cout << expect.c_str() << endl;
    return 0;
}