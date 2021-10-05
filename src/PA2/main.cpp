#include "Lexer.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

int main(int argc, char** argv) {
    if (argc == 1) {
        cerr << "Usage: cool_lexer [file.cl]" << endl;
        return 1;
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
    } else {
        cerr << "File " << fileName << " wasn't found" << endl;
        return 1;
    }
    return 0;
}