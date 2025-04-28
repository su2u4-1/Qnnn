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
/*                佛祖保佑       永無BUG                 */
/*******************************************************/

#include "../include/compiler.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/qlib.h"

using namespace std;

struct arguments {
    fs::path program_name = "";
    vector<fs::path> files = {};
    vector<string> flags = {};
    fs::path output_ast_file = "";
    int output_ast_type = -1;
    /*
    no output ast = -1
    .ast = 0
    _ast.json = 1
    */
};

vector<string> read_file(const fs::path& file_name) {
    ifstream input;
    input.open(file_name);
    if (input.fail())
        throw runtime_error("Error: Could not open file " + file_name.string());
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

    int state = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            args.flags.push_back(argv[i]);
            if (string(argv[i]) == "--output-ast" || string(argv[i]) == "-oa") {
                args.output_ast_type = 0;
                args.output_ast_file = args.program_name.replace_extension(".ast");
            } else if (string(argv[i]) == "--output-ast-json" || string(argv[i]) == "-oaj") {
                args.output_ast_type = 1;
                args.output_ast_file = args.program_name.replace_filename(args.program_name.filename().string() + "_ast.json");
            } else if (string(argv[i]) == "--output-ast-none" || string(argv[i]) == "-oan") {
                args.output_ast_type = -1;
            } else if (string(argv[i]) == "--help" || string(argv[i]) == "-h") {
                cout << HELP_DOCS;
                exit(0);
            } else if (string(argv[i]) == "--output" || string(argv[i]) == "-o") {
                state = 1;
            }
        } else {
            fs::path t = fs::absolute(argv[i]);
            if (state == 0) {
                if (fs::exists(t) && t.extension() == ".qn") {
                    args.files.push_back(t);
                } else {
                    cerr << "Error: File " << t << " does not exist or is not a .qn file" << endl;
                    exit(1);
                }
            } else if (state == 1) {
                args.program_name = t.string().substr(0, t.string().find_last_of('.'));
                state = 0;
            }
        }
    }
    return args;
}

string ast_to_json(shared_ptr<Node> node) {
    stringstream output;
    output << "{";
    output << "\"type\":\"" << node->type << "\",";
    output << "\"value\":{";
    for (const auto& [k, v] : node->value)
        output << '"' << k << "\":\"" << v << "\",";
    output << "},";
    output << "\"children\":[";
    for (const shared_ptr<Node>& child : node->children)
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

string output_ast(const shared_ptr<Node> node, int ident) {
    stringstream output;
    output << string(ident * 4, ' ') << node->type << " (";
    for (const auto& [k, v] : node->value)
        output << k << ": " << v << ", ";
    if (node->children.size() == 0)
        output << ") {}" << endl;
    else {
        output << ")" << endl;
        output << string(ident * 4, ' ') << "{" << endl;
        ident++;
        for (const auto& child : node->children)
            output << output_ast(child, ident);
        ident--;
        output << string(ident * 4, ' ') << "}" << endl;
    }
    return output.str();
}

int main(int argc, char* argv[]) {
    try {
        cout << "base path: " << BASEPATH << endl;
        cout << "parse arguments" << endl;
        arguments args = parse_arguments(argc, argv);
        if (args.files.size() == 0) {
            cerr << "Error: No input files" << endl;
            return 1;
        }
        cout << "output_ast_file: " << args.output_ast_file << endl;
        string output = "";
        for (const fs::path& file : args.files) {
            cout << "read source code [" << file << "]" << endl;
            vector<string> source_code;
            try {
                source_code = read_file(file);
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                return 1;
            }
            source_code_setitem(file, source_code);

            cout << "lexing [" << file << "]" << endl;
            vector<shared_ptr<Token>> tokens;
            try {
                tokens = lexer(source_code, file);
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                return 1;
            }

            cout << "parsing [" << file << "]" << endl;
            shared_ptr<Node> ast;
            try {
                ast = Parser(tokens, file).parse();
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                return 1;
            }

            if (args.output_ast_type > -1) {
                cout << "outputting AST to ";
                if (args.output_ast_type == 0) {
                    cout << "ast file [" << args.output_ast_file << "]" << endl;
                    output += output_ast(ast, 0);
                } else if (args.output_ast_type == 1) {
                    cout << "json file [" << args.output_ast_file << "]" << endl;
                    output += remove_json_trailing_comma(ast_to_json(ast));
                }
            }

            cout << "compiling [" << file << "]" << endl;
            Compiler compiler(*ast);  // t
            compiler.compile();       // t
        }
        ofstream output_file(args.output_ast_file);
        if (output_file.fail())
            throw runtime_error("Error: Could not open file " + args.output_ast_file.string());
        output_file << output;
        output_file.close();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
