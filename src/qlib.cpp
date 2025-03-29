#include "../include/qlib.h"

vector<string> keywords = {"arr", "as", "attr", "bool", "break", "char", "class", "constant", "continue", "dict", "elif", "else", "false", "float", "for", "fpointer", "function", "global", "if", "import", "in", "int", "method", "NULL", "pointer", "public", "range", "return", "static", "str", "true", "tuple", "type", "var", "void", "while"};
vector<string> symbols = {"(", ")", "[", "]", "{", "}", ",", ";", ".", "+", "-", "*", "/", "%", "<", ">", "&", "|", "=", "@", "^", "!", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "**", "<<", ">>"};
map<string, vector<string>> source_code_map = map<string, vector<string>>();

bool is_keyword(string word) {
    for (const string& i : keywords) {
        if (word == i) {
            return true;
        }
    }
    return false;
}

bool is_symbol(string word) {
    for (const string& i : symbols) {
        if (word == i) {
            return true;
        }
    }
    return false;
}

bool is_symbol(char c) {
    return is_symbol(string(1, c));
}

void error(string msg, string file_name, pair<int, int> pos, string source_code) {
    ostringstream oss;
    oss << "File " << file_name << ", line " << pos.first << ", in " << pos.second << "\n"
        << msg << "\n"
        << source_code << string(pos.second, ' ') << "^";
    throw runtime_error(oss.str());
}

// Token
Token::Token(string type, string value, string file_name, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->file_name = file_name;
    this->line = pos.first;
    this->column = pos.second;
}

Token::Token(string type, string value) {
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
Tokens::Tokens(string type, vector<string> value) {
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
Node::Node(string type, map<string, string> value, vector<Node> children) {
    this->type = type;
    this->value = value;
    this->children = children;
}

Node::Node(string type, map<string, string> value, Node child) {
    this->type = type;
    this->value = value;
    this->children.push_back(child);
}

Node::Node(string type, map<string, string> value) {
    this->type = type;
    this->value = value;
    this->children = vector<Node>();
}

Node::Node(string type) {
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
    for (const pair<string, string>& i : value) {
        t += i.first + ": " + i.second + ", ";
    }
    t += "}";
    for (Node i : children) {
        t += ", " + i.toString();
    }
    t += ")";
    return t;
}
