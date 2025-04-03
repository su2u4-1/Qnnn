#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/qlib.h"

using namespace std;

vector<string> read_file(string file_name) {
    ifstream input;
    input.open(file_name);
    if (input.fail())
        throw runtime_error("Error: Could not open file " + file_name);
    vector<string> source_code;
    string line;
    while (getline(input, line)) {
        if (line.size() > 0) {
            if (line.back() == '\r')
                line.pop_back();
            if (line.back() != '\n')
                line.append("\n");
        }
        source_code.push_back(line);
    }
    input.close();
    return source_code;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string file_name = argv[1];
    if (file_name.find(".qn") == string::npos) {
        cerr << "Error: File must have .qn extension" << endl;
        return 1;
    }

    vector<string> source_code;
    try {
        source_code = read_file(file_name);
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
    source_code_setitem(file_name, source_code);

    try {
        vector<Token> tokens = lexer(source_code, file_name);
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
