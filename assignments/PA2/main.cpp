#include "Lexer.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

std::stringstream getExpected(const std::string& lexerPath, const std::string& fileName) {
    std::system((lexerPath + " " + fileName + " > actual.txt").c_str());
    ifstream actualFile("actual.txt");
    stringstream actualResult;
    actualResult << actualFile.rdbuf();
    actualFile.close();
    std::remove("actual.txt");

    return actualResult;
}

std::stringstream getActual(const std::string& fileName) {
    ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("File " + fileName + " wasn't found");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    stringstream expectedResult;
    auto lexer = Lexer(buffer.str());
    expectedResult << "#name \"" << fileName << "\"" << endl;
    while (lexer.hasNext()) {
        expectedResult << lexer.next().toString() << endl;
    }
    return expectedResult;
}

int main(int argc, char** argv) {
    auto lexerPath = std::string(argv[1]);
    auto fileName = std::string(argv[2]);

    auto expectedResult = getExpected(lexerPath, fileName);
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
    return 0;
}