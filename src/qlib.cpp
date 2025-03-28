#include "../include/qlib.h"

vector<string> keywords = {"arr", "as", "attr", "bool", "break", "char", "class", "constant", "continue", "dict", "elif", "else", "false", "float", "for", "fpointer", "function", "global", "if", "import", "in", "int", "method", "NULL", "pointer", "public", "range", "return", "static", "str", "true", "tuple", "type", "var", "void", "while"};
vector<string> symbols = {"(", ")", "[", "]", "{", "}", ",", ";", ".", "+", "-", "*", "/", "%", "<", ">", "&", "|", "=", "@", "^", "!", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=", "%=", "**", "<<", ">>"};

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

// Token
Token::Token(string type, string value, string file_name, pair<int, int> pos) {
    this->type = type;
    this->value = value;
    this->file_name = file_name;
    this->line = pos.first;
    this->column = pos.second;
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
