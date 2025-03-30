#include "../include/qlib.h"

unordered_set<string> keywords = {"arr", "as", "attr", "bool", "break", "char", "class", "constant", "continue", "dict", "elif", "else", "false", "float", "for", "fpointer", "function", "global", "if", "import", "in", "int", "method", "NULL", "pointer", "public", "range", "return", "static", "str", "true", "tuple", "type", "var", "void", "while"};
unordered_set<string> symbols = {"(", ")", "[", "]", "{", "}", ",", ";", ".", "+", "-", "*", "/", "%", "<", ">", "&", "|", "=", "@", "^", "!", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "**", "<<", ">>"};
map<string, vector<string>> source_code_map = map<string, vector<string>>();

vector<string> STDLIB = {"math", "list", "random", "io", "time"};
vector<string> BUILTINTYPE = {"int", "float", "bool", "char", "str", "void", "dict", "arr", "tuple", "fpointer", "pointer"};

bool is_keyword(const string& word) {
    return keywords.count(word) > 0;
}

bool is_symbol(const string& word) {
    return symbols.count(word) > 0;
}

bool is_symbol(char c) {
    return is_symbol(string(1, c));
}

void error(const string& msg, const string& file_name, pair<int, int> pos, const string& source_code) {
    ostringstream oss;
    oss << "File " << file_name << ", line " << pos.first << ", in " << pos.second << "\n"
        << msg << "\n"
        << source_code << string(pos.second, ' ') << "^";
    throw runtime_error(oss.str());
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
    return "Token(" + type + ", " + value + ", " + to_string(line) + ", " + to_string(column) + ")";
}

bool Token::operator==(const Token& other) const {
    return type == other.type && value == other.value;
}

bool Token::operator==(const Tokens& other) const {
    if (type != other.type) {
        return false;
    }
    for (const string& i : other.value) {
        if (value == i) {
            return true;
        }
    }
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
    for (const string& i : value) {
        t += ", " + i;
    }
    return "Tokens(" + type + t + ")";
}

bool Tokens::operator==(const Token& other) const {
    if (type != other.type) {
        return false;
    }
    for (const string& i : value) {
        if (other.value == i) {
            return true;
        }
    }
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
    for (const auto& [key, val] : value) {
        t += key + ": " + val + ", ";
    }
    t += "}";
    for (Node i : children) {
        t += ", " + i.toString();
    }
    t += ")";
    return t;
}
