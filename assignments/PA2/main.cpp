#include "Lexer.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

int main(int argc, char** argv) {
    if (argc == 1) {
        cout << "Usage: cool_lexer [file.cl]" << endl;
        return 0;
    }

    auto fileName = argv[1];
    ifstream file(fileName);

    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto lexer = Lexer(buffer.str());
        cout << "#name " << std::filesystem::path(fileName).filename() << endl;
        while (lexer.hasNext()) {
            cout << lexer.next().toString() << endl;
        }
    }
    return 0;
}