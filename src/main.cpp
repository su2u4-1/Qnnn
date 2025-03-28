#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/qlib.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string file_name = argv[1];
    ifstream input;
    if (file_name.find(".qn") == string::npos) {
        cerr << "Error: File must have .qn extension" << endl;
        return 1;
    }
    input.open(file_name);
    if (input.fail()) {
        cerr << "Error: Could not open file " << file_name << endl;
        return 1;
    }
    vector<string> source_code;
    string line;
    while (getline(input, line)) {
        line += "\n";
        source_code.push_back(line);
    }
    input.close();

    auto t = lexer(source_code, file_name);
    vector<Token> tokens = t.first;
    string error_msg = t.second;
    if (error_msg != "") {
        cerr << error_msg << endl;
        return 1;
    }

    return 0;
}
