#include "../include/qlib.h"

unordered_set<string> _KEYWORD = {"arr", "as", "attr", "break", "class", "const", "continue", "elif", "else", "false", "for", "func", "if", "import", "in", "int", "method", "NULL", "op", "public", "return", "static", "true", "type", "var", "void", "while"};
unordered_set<string> _SYMBOLS = {"(", ")", "[", "]", "{", "}", ",", ";", ".", "+", "-", "*", "/", "%", "<", ">", "&", "|", "=", "@", "^", "!", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "**", "<<", ">>"};
map<fs::path, vector<string>> source_code_map = map<fs::path, vector<string>>();
string HELP_DOCS =
    "Usage: qlib <filename> [options]\n"
    "Options:\n"
    "  -oa,  --output-ast        Output AST to file\n"
    "  -oaj, --output-ast-json   Output AST to JSON file\n"
    "  -oan, --output-ast-none   No output AST\n"
    "  -h,   --help              Show this help message\n";
fs::path BASEPATH = path_processing(fs::absolute(fs::path(__FILE__).parent_path().parent_path()));
fs::path STDLIBPATH = BASEPATH / "stdlib";
string VERSION = "x.y.z";  // Placeholder for version

vector<string> STDLIB = {"math", "list", "random", "io", "time"};
vector<string> BUILTINTYPE = {"int", "void", "NULL", "arr", "type"};
vector<string> OPERATOR = {"+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "&", "|", "=", "+=", "-=", "*=", "/=", "%=", "**", "@", "^", "<<", ">>", "!"};

bool is_keyword(const string& word) {
    return find(_KEYWORD.begin(), _KEYWORD.end(), word) != _KEYWORD.end();
}

bool is_symbol(const string& word) {
    return find(_SYMBOLS.begin(), _SYMBOLS.end(), word) != _SYMBOLS.end();
}

bool is_symbol(char c) {
    return is_symbol(string(1, c));
}

int operator_precedence(const string& op) {
    if (op == "!" || op == "-" || op == "@" || op == "^" || op == "**")
        return 1;
    else if (op == "*" || op == "/" || op == "%")
        return 2;
    else if (op == "+" || op == "-")
        return 3;
    else if (op == "<<" || op == ">>")
        return 4;
    else if (op == "<" || op == "<=" || op == ">" || op == ">=")
        return 5;
    else if (op == "==" || op == "!=")
        return 6;
    else if (op == "&" || op == "|")
        return 7;
    else if (op == "&&" || op == "||")
        return 8;
    else if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=")
        return 9;
    return -1;
}

bool is_term(const Token& token) {
    if (token.type == "identifier")
        return true;
    else if (token == Tokens("keyword", BUILTINTYPE))
        return true;
    else if (token == Tokens("symbol", {"(", "[", "{", "@", "^", "!", "-"}))
        return true;
    else if (token == Tokens("keyword", {"true", "false", "NULL"}))
        return true;
    else if (token.type == "int" || token.type == "float" || token.type == "char" || token.type == "str")
        return true;
    else if (token == Tokens("keyword", {"arr", "dict", "tuple", "pointer", "type", "range"}))
        return true;
    else
        return false;
}

void error(const string& msg, const fs::path& file_path, pair<int, int> pos, const string& source_code) {
    throw runtime_error("File " + path_processing(file_path) + ", line " + to_string(pos.first) + ", in " + to_string(pos.second) + "\n" + msg + "\n" + source_code + string(pos.second, ' ') + "^");
}

string path_processing(const fs::path& file_path) {
    string file_name = file_path.string();
    if (PLATFORM_NAME == "windows") {
        if (file_name[0] == '/' && (file_name[1] > 'a' && file_name[1] < 'z')) {
            file_name = string(1, file_name[1] - 32) + ":" + file_name.substr(2);
        }
    }
    while (file_name.find("/./") != string::npos) {
        file_name.replace(file_name.find("/./"), 3, "/");
    }
    replace(file_name.begin(), file_name.end(), '\\', '/');
    return file_name;
}

// Log
Log::Log(const string& file_name) {
    this->file_name = file_name;
    this->indent = 0;
    this->t = "";
    ofstream clear_file(file_name);
    clear_file.clear();
    clear_file.close();
}

Log::Log() {
    this->file_name = "./temp/log.log";
    this->indent = 0;
    this->t = "";
}

void Log::log_msg(const string& msg, const int mode) {
    string s, o;
    if (mode == 0)
        s = "(in) " + msg + "()";
    else if (mode == 1)
        s = "(out) " + msg + "()";
    else if (mode == 2) {
        t = msg;
        s = "(get) " + msg;
    } else if (mode == 3)
        s = "(stage) " + msg;
    else if (mode == 5) {
        t = msg;
        s = "(rollback) " + msg;
    } else
        s = "(error) " + msg;
    if (s.find("(in)") != string::npos)
        indent++;
    for (int j = 0; j < indent; j++)
        o += "    ";
    o += s + "\n";
    if (s.find("(out)") != string::npos)
        indent--;
    ofstream log_file(file_name, ios::app);
    if (log_file.fail())
        throw runtime_error("Error: Unable to open log file: " + file_name);
    else {
        log_file << o;
        log_file.close();
    }
}

void Log::start_call_stack() {
    ofstream log_file(file_name, ios::app);
    if (log_file.fail())
        throw runtime_error("Error: Unable to open log file: " + file_name);
    else {
        log_file << "\nCall stack start\n";
        log_file.close();
        indent = 0;
        t = "";
    }
}

void Log::end_call_stack() {
    ofstream log_file(file_name, ios::app);
    if (log_file.fail())
        throw runtime_error("Error: Unable to open log file: " + file_name);
    else
        log_file << "Call stack end\n\n";
}

void source_code_setitem(fs::path file_name, vector<string> source_code) {
    source_code_map[file_name] = source_code;
}

string source_code_getitem(fs::path file_name, int line) {
    return source_code_map[file_name][line];
}

// Token
Token::Token(const string& type, const string& value, const fs::path& file_name, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->file_name = file_name;
    this->pos = pos;
}

Token::Token(const string& type, const string& value) {
    this->type = type;
    this->value = value;
    this->file_name = fs::path();
    this->pos = pair<int, int>(-1, -1);
}

Token::Token() {
    this->type = "None";
    this->value = "None";
    this->file_name = fs::path();
    this->pos = pair<int, int>(-1, -1);
}

string Token::toString() const {
    return type + "< " + value + " > (" + to_string(pos.first) + ", " + to_string(pos.second) + ")";
}

bool Token::operator==(const Token& other) const {
    return type == other.type && value == other.value;
}

bool Token::operator==(const Tokens& other) const {
    if (type != other.type)
        return false;
    for (const string& i : other.value)
        if (value == i) return true;
    return false;
}

bool Token::operator!=(const Token& other) const {
    return !(*this == other);
}

bool Token::operator!=(const Tokens& other) const {
    return !(*this == other);
}

// Tokens
Tokens::Tokens(const string& type, const vector<string>& value) {
    this->type = type;
    this->value = value;
}

string Tokens::toString() const {
    string t = "";
    for (const string& i : value)
        t += ", " + i;
    return "Tokens(" + type + t + ")";
}

bool Tokens::operator==(const Token& other) const {
    if (type != other.type)
        return false;
    for (const string& i : value)
        if (other.value == i) return true;
    return false;
}

bool Tokens::operator!=(const Token& other) const {
    return !(*this == other);
}

// Node
Node::Node(const string& type, const map<string, string>& value, const vector<shared_ptr<Node>>& children, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->children = children;
    this->pos = pos;
}

Node::Node(const string& type, const map<string, string>& value, const shared_ptr<Node>& child, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->children.push_back(child);
    this->pos = pos;
}

Node::Node(const string& type, const map<string, string>& value, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->children = vector<shared_ptr<Node>>();
    this->pos = pos;
}

Node::Node(const string& type, const map<string, string>& value) {
    this->type = type;
    this->value = value;
    this->children = vector<shared_ptr<Node>>();
    this->pos = pair<int, int>(-1, -1);
}

Node::Node(const string& type, pair<int, int> pos) {
    this->type = type;
    this->value = map<string, string>();
    this->children = vector<shared_ptr<Node>>();
    this->pos = pos;
}

Node::Node(const string& type) {
    this->type = type;
    this->value = map<string, string>();
    this->children = vector<shared_ptr<Node>>();
    this->pos = pair<int, int>(-1, -1);
}

Node::Node() {
    this->type = "None";
    this->value = map<string, string>();
    this->children = vector<shared_ptr<Node>>();
    this->pos = pair<int, int>(-1, -1);
}

string Node::toString() const {
    string t = type + "({";
    for (const auto& [key, val] : value)
        t += key + ": " + val + ", ";
    t += "}";
    for (const shared_ptr<Node>& i : children)
        t += ", " + i->toString();
    t += ")";
    return t;
}
