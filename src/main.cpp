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
    try {
        cout << "0. check args" << endl;
        if (argc < 2) {
            cerr << "Usage: " << argv[0] << " <filename>" << endl;
            return 1;
        }
        string file_name = argv[1];
        string output_name = file_name.substr(0, file_name.find_last_of('.')) + ".out";

        cout << "1. check file extension " << file_name << endl;
        if (file_name.find(".qn") == string::npos) {
            cerr << "Error: File must have .qn extension" << endl;
            return 1;
        }

        cout << "2. start read source code" << endl;
        vector<string> source_code;
        try {
            source_code = read_file(file_name);
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }
        // for (const string& line : source_code)
        //     cout << line;
        source_code_setitem(file_name, source_code);

        cout << "3. read completed, start lexing" << endl;
        vector<Token> tokens;
        try {
            tokens = lexer(source_code, file_name);
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }
        // for (Token& token : tokens)
        //     cout << token.toString() << endl;

        cout << "4. tokens generated, start parsing" << endl;
        Node ast;
        try {
            ast = Parser(tokens, file_name).parse();
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }
        ofstream output_file(output_name);
        output_file << ast.toString();
        output_file.close();

        cout << "5. parsing completed, start compile" << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
