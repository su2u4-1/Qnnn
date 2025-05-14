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
    fs::path program_name = fs::path("output");
    vector<fs::path> files = {};
    vector<string> flags = {};
    fs::path output_ast_file = fs::path("output.ast");
    int output_ast_type = -1;
    fs::path compile_file = fs::path("output.vm");
    bool compile = false;
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
        throw runtime_error("Could not open file " + file_name.string());
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
        cerr << "Usage: " << path_processing(argv[0]) << " <filename>" << endl;
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
                args.output_ast_file = path_processing(args.program_name.replace_extension(".ast"));
            } else if (string(argv[i]) == "--output-ast-json" || string(argv[i]) == "-oaj") {
                args.output_ast_type = 1;
                args.output_ast_file = path_processing(args.program_name.replace_filename(args.program_name.filename().string() + "_ast").replace_extension(".json"));
            } else if (string(argv[i]) == "--output-ast-none" || string(argv[i]) == "-oan")
                args.output_ast_type = -1;
            else if (string(argv[i]) == "--help" || string(argv[i]) == "-h") {
                cout << HELP_DOCS;
                exit(0);
            } else if (string(argv[i]) == "--output" || string(argv[i]) == "-o")
                state = 1;
            else if (string(argv[i]) == "--stdlibpath" || string(argv[i]) == "-sp")
                state = 2;
            else if (string(argv[i]) == "--compile" || string(argv[i]) == "-c") {
                args.compile_file = path_processing(args.program_name.replace_extension(".vm"));
                args.compile = true;
            } else if (string(argv[i]) == "--version" || string(argv[i]) == "-v") {
                cout << "Version: " << VERSION << endl;
                exit(0);
            } else
                throw "Error: Unknown argument " + string(argv[i]);
        } else {
            fs::path t = path_processing(fs::absolute(argv[i]));
            if (state == 0) {
                if (fs::exists(t) && t.extension() == ".qn")
                    args.files.push_back(t);
                else {
                    cerr << "Error: File " << t << " does not exist or is not a .qn file" << endl;
                    exit(1);
                }
            } else if (state == 1) {
                args.program_name = path_processing(t.parent_path() / t.stem());
                state = 0;
            } else if (state == 2)
                STDLIBPATH = t;
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
        Log log("./temp/log.log");
        log.log_msg("base path: " + BASEPATH.string(), 3);
        log.log_msg("parse arguments", 3);
        arguments args = parse_arguments(argc, argv);
        if (args.files.size() == 0)
            throw runtime_error("No input files");
        string output = "";
        ofstream clear_file(args.compile_file);
        clear_file.clear();
        clear_file.close();
        for (int i = 0; i < args.files.size(); i++) {
            const fs::path& file = args.files[i];
            log.log_msg("read source code [" + file.string() + "]", 3);
            vector<string> source_code;
            try {
                source_code = read_file(file);
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                exit(1);
            }
            source_code_setitem(file, source_code);

            log.log_msg("lexing [" + file.string() + "]", 3);
            vector<shared_ptr<Token>> tokens;
            try {
                tokens = lexer(source_code, file);
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                exit(1);
            }

            log.log_msg("parsing [" + file.string() + "]", 3);
            shared_ptr<Node> ast;
            log.start_call_stack();
            try {
                ast = Parser(tokens, file, log).parse();
            } catch (const runtime_error& e) {
                cerr << e.what() << endl;
                exit(1);
            }
            log.end_call_stack();

            if (args.output_ast_type > -1) {
                string t = "outputting AST to ";
                if (args.output_ast_type == 0) {
                    log.log_msg(t + "ast file [" + args.output_ast_file.string() + "]", 3);
                    output += output_ast(ast, 0);
                } else if (args.output_ast_type == 1) {
                    log.log_msg(t + "json file [" + args.output_ast_file.string() + "]", 3);
                    if (output.empty())
                        output = "[" + ast_to_json(ast);
                    else
                        output += ast_to_json(ast);
                }
                ofstream output_file(args.output_ast_file);
                if (output_file.fail())
                    throw runtime_error("Could not open file " + args.output_ast_file.string());
                if (args.output_ast_type == 1)
                    output_file << remove_json_trailing_comma(output + "]");
                else
                    output_file << output;
                output_file.close();
            }

            log.log_msg("compiling [" + file.string() + "]", 3);
            Compiler compiler(*ast, log);
            log.start_call_stack();
            vector<string> target_code = compiler.compile();
            log.end_call_stack();
            for (const fs::path& i : compiler.import_list) {
                if (find(args.files.begin(), args.files.end(), i) == args.files.end()) {
                    if (fs::exists(i) && i.extension() == ".qn") {
                        log.log_msg("importing [" + i.string() + "]", 3);
                        args.files.push_back(i);
                    } else {
                        throw runtime_error("File " + i.string() + " does not exist or is not a .qn file");
                    }
                }
            }

            if (args.compile) {
                log.log_msg("Outputing compiled file to [" + args.compile_file.string() + "]", 3);
                ofstream output_file(args.compile_file, ios::app);
                if (output_file.fail())
                    throw runtime_error("Could not open file " + args.compile_file.string());
                for (const string& i : target_code)
                    output_file << i << "\n";
                output_file.close();
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }
    return 0;
}
