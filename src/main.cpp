/*******************************************************/
/*                       _ooOoo                        */
/*                      o8888888o                      */
/*                      88" . "88                      */
/*                      (| -_- |)                      */
/*                      O\  =  /O                      */
/*                    ___/`---'\____                   */
/*                 .'  \\|     |//  `.                 */
/*                /  \\|||  :  |||//  \                */
/*               /  _||||| -:- |||||_  \               */
/*               |   | \\\  -  /// |   |               */
/*               | \_|  ''\---/''  |   |               */
/*               \  .-\__       __/-.  /               */
/*             ___`. .'  /--.--\ `. . __               */
/*          ."" '<  `.___\_<|>_/__.'  >'"".            */
/*         | | :  `- \`.;`\ _ /`;.`/ - ` : | |         */
/*         \  \ `-.   \_ __\ /__ _/   .-` /  /         */
/*    ======`-.____`-.___\_____/___.-`____.-'======    */
/*                       `=---='                       */
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/*                佛祖保佑       永無BUG                */
/*******************************************************/

#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/qlib.h"

using namespace std;

struct arguments {
    string program_name = "";
    string source_code_file = "";
    vector<string> flags = {};
    string output_ast_file = "";
    int output_ast_type = -1;
    /*
    no output ast = -1
    .ast = 0
    _ast.json = 1
    */
};

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

arguments parse_arguments(int argc, char* argv[]) {
    arguments args;
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        exit(1);
    }
    if (string(argv[1]) == "--help" || string(argv[1]) == "-h") {
        cout << HELP_DOCS;
        exit(0);
    }
    args.source_code_file = argv[1];
    args.program_name = args.source_code_file.substr(0, args.source_code_file.find_last_of('.'));
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') {
            args.flags.push_back(argv[i]);
            if (string(argv[i]) == "--output-ast" || string(argv[i]) == "-oa") {
                args.output_ast_type = 0;
                args.output_ast_file = args.program_name + ".ast";
            } else if (string(argv[i]) == "--output-ast-json" || string(argv[i]) == "-oaj") {
                args.output_ast_type = 1;
                args.output_ast_file = args.program_name + "_ast.json";
            } else if (string(argv[i]) == "--output-ast-none" || string(argv[i]) == "-oan") {
                args.output_ast_type = -1;
            } else if (string(argv[i]) == "--help" || string(argv[i]) == "-h") {
                cout << HELP_DOCS;
                exit(0);
            }
        }
    }
    if (args.source_code_file.find(".qn") == string::npos) {
        cerr << "Error: File name must end with .qn" << endl;
        exit(1);
    }
    return args;
}

string ast_to_json(const Node& node) {
    stringstream output;
    output << "{";
    output << "\"type\":\"" << node.type << "\",";
    output << "\"value\":{";
    for (const auto& [k, v] : node.value)
        output << '"' << k << "\":\"" << v << "\",";
    output << "},";
    output << "\"children\":[";
    for (const auto& child : node.children)
        output << ast_to_json(child);
    output << "]";
    output << "},";
    return output.str();
}

string remove_json_trailing_comma(const string& json) {
    char n, p;
    bool str = false;
    string result;
    for (int i = 0; i < json.size(); i++) {
        p = n;
        n = json[i];
        if (n == '"')
            str = !str;
        if (!str && p == ',' && (n == '}' || n == ']'))
            result[result.size() - 1] = n;
        else
            result += n;
    }
    if (result[result.size() - 1] == ',')
        result[result.size() - 1] = ' ';
    return result;
}

string output_ast(const Node& node, int ident) {
    stringstream output;
    output << string(ident * 4, ' ') << node.type << " (";
    for (const auto& [k, v] : node.value) {
        output << k << ": " << v << ", ";
    }
    if (node.children.size() == 0)
        output << ") {}" << endl;
    else {
        output << ")" << endl;
        output << string(ident * 4, ' ') << "{" << endl;
        ident++;
        for (const auto& child : node.children) {
            output << output_ast(child, ident);
        }
        ident--;
        output << string(ident * 4, ' ') << "}" << endl;
    }
    return output.str();
}

int main(int argc, char* argv[]) {
    try {
        cout << "prase arguments" << endl;
        arguments args = parse_arguments(argc, argv);

        cout << "read source code [" << args.source_code_file << "]" << endl;
        vector<string> source_code;
        try {
            source_code = read_file(args.source_code_file);
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }
        source_code_setitem(args.source_code_file, source_code);

        cout << "lexing [" << args.source_code_file << "]" << endl;
        vector<Token> tokens;
        try {
            tokens = lexer(source_code, args.source_code_file);
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }

        cout << "parsing [" << args.source_code_file << "]" << endl;
        Node ast;
        try {
            ast = Parser(tokens, args.source_code_file).parse();
        } catch (const runtime_error& e) {
            cerr << e.what() << endl;
            return 1;
        }

        if (args.output_ast_type > -1) {
            cout << "outputting AST to ";
            string output;
            if (args.output_ast_type == 0) {
                cout << "ast file [" << args.output_ast_file << "]" << endl;
                output = output_ast(ast, 0);
            } else if (args.output_ast_type == 1) {
                cout << "json file [" << args.output_ast_file << "]" << endl;
                output = remove_json_trailing_comma(ast_to_json(ast));
            }
            ofstream output_file(args.output_ast_file);
            if (output_file.fail())
                throw runtime_error("Error: Could not open file " + args.output_ast_file);
            output_file << output;
            output_file.close();
        }

        cout << "compiling [" << args.source_code_file << "]" << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
