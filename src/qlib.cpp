#include "../include/qlib.h"

unordered_set<string> _KEYWORD = {"arr", "as", "attr", "bool", "break", "char", "class", "const", "continue", "dict", "elif", "else", "false", "float", "for", "func", "global", "if", "import", "in", "int", "method", "NULL", "pointer", "public", "range", "return", "static", "str", "true", "tuple", "type", "var", "void", "while"};
unordered_set<string> _SYMBOLS = {"(", ")", "[", "]", "{", "}", ",", ";", ".", "+", "-", "*", "/", "%", "<", ">", "&", "|", "=", "@", "^", "!", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "**", "<<", ">>"};
map<string, vector<string>> source_code_map = map<string, vector<string>>();
string HELP_DOCS =
    "Usage: qlib <filename> [options]\n"
    "Options:\n"
    "  -oa,  --output-ast        Output AST to file\n"
    "  -oaj, --output-ast-json   Output AST to JSON file\n"
    "  -oan, --output-ast-none   No output AST\n"
    "  -h,   --help              Show this help message\n";

vector<string> STDLIB = {"math", "list", "random", "io", "time"};
vector<string> BUILTINTYPE = {"int", "float", "bool", "char", "str", "void", "dict", "arr", "tuple", "pointer", "type", "range"};
vector<string> OPERATOR = {"+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "&", "|", "=", "+=", "-=", "*=", "/=", "%=", "**", "@", "^", "<<", ">>", "!"};
vector<string> call_stack = {};

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

void error(const string& msg, const string& file_name, pair<int, int> pos, const string& source_code) {
    ostringstream oss;
    oss << get_call_stack();
    oss << "File " << file_name << ", line " << pos.first << ", in " << pos.second << "\n"
        << msg << "\n"
        << source_code << string(pos.second, ' ') << "^";
    throw runtime_error(oss.str());
}

string get_call_stack() {
    string o = "Call stack (most recent call last):\n";
    int indent = 0;
    for (const string& i : call_stack) {
        if (i.find("(in)") != string::npos)
            indent++;
        for (int j = 0; j < indent; j++)
            o += "    ";
        o += i + "\n";
        if (i.find("(out)") != string::npos)
            indent--;
    }
    return o;
}

void add_call_stack(const string& str, const int mode) {
    static string t;
    if (mode == 0)
        call_stack.push_back("\033[32m(in) \033[0m" + str + "()");
    else if (mode == 1)
        call_stack.push_back("\033[34m(out) \033[0m" + str + "()");
    else if (mode == 2) {
        if (t == str)
            return;
        t = str;
        call_stack.push_back("(get) " + str);
    } else
        call_stack.push_back("(error) " + str);
}

void source_code_setitem(string file_name, vector<string> source_code) {
    source_code_map[file_name] = source_code;
}

string source_code_getitem(string file_name, int line) {
    return source_code_map[file_name][line];
}

// Token
Token::Token(const string& type, const string& value, const string& file_name, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->file_name = file_name;
    this->line = pos.first;
    this->column = pos.second;
}

Token::Token(const string& type, const string& value) {
    this->type = type;
    this->value = value;
    this->file_name = "";
    this->line = -1;
    this->column = -1;
}

Token::Token() {
    this->type = "None";
    this->value = "None";
    this->file_name = "";
    this->line = -1;
    this->column = -1;
}

string Token::toString() {
    return type + "< " + value + " > (" + to_string(line) + ", " + to_string(column) + ")";
    // return "Token(" + type + ", " + value + ", " + to_string(line) + ", " + to_string(column) + ")";
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

string Tokens::toString() {
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
Node::Node(const string& type, const map<string, string>& value, const vector<Node>& children) {
    this->type = type;
    this->value = value;
    this->children = children;
}

Node::Node(const string& type, const map<string, string>& value, const Node& child) {
    this->type = type;
    this->value = value;
    this->children.push_back(child);
}

Node::Node(const string& type, const map<string, string>& value) {
    this->type = type;
    this->value = value;
    this->children = vector<Node>();
}

Node::Node(const string& type) {
    this->type = type;
    this->value = map<string, string>();
    this->children = vector<Node>();
}

Node::Node() {
    this->type = "None";
    this->value = map<string, string>();
    this->children = vector<Node>();
}

string Node::toString() {
    string t = type + "({";
    for (const auto& [key, val] : value)
        t += key + ": " + val + ", ";
    t += "}";
    for (Node i : children)
        t += ", " + i.toString();
    t += ")";
    return t;
}
