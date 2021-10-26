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

    auto file_name = argv[1];
    ifstream file(file_name);

    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto lexer = lexer::Lexer(buffer.str());
        cout << "#name " << std::filesystem::path(file_name).filename() << endl;
        while (lexer.HasNext()) {
            cout << lexer.Next().ToString() << endl;
        }
    } else {
        cerr << "File " << file_name << " wasn't found" << endl;
        return 1;
    }
    return 0;
}