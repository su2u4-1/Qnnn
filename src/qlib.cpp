#include "../include/qlib.h"

// Token 類別方法實現
Token::Token(int type, string value, int line, int column) {
    this->type = type;
    this->value = value;
    this->line = line;
    this->column = column;
}

string Token::toString() {
    return "Token(" + to_string(type) + ", " + value + ", " + to_string(line) + ", " + to_string(column) + ")";
}

bool Token::operator==(const Token& other) const {
    return type == other.type && value == other.value;
}

bool Token::operator!=(const Token& other) const {
    return !(*this == other);
}

// Tokens 類別方法實現
Tokens::Tokens(int type, vector<string> value) {
    this->type = type;
    this->value = value;
}

string Tokens::toString() {
    string t = "";
    for (const string& i : value) {
        t += ", " + i;
    }
    return "Tokens(" + to_string(type) + t + ")";
}
