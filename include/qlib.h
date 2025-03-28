#ifndef QLIB_H
#define QLIB_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

bool is_keyword(string word);
bool is_symbol(string word);
bool is_symbol(char c);

class Token {
   public:
    Token(string type, string value, string file_name, pair<int, int> pos);
    string type;
    string value;
    string file_name;
    int line;
    int column;
    string toString();
    bool operator==(const Token& other) const;
    bool operator==(const Tokens& other) const;
    bool operator!=(const Token& other) const;
    bool operator!=(const Tokens& other) const;
};

class Tokens {
   public:
    Tokens(string type, vector<string> value);
    string type;
    vector<string> value;
    string toString();
    bool Tokens::operator==(const Token& other) const;
    bool Tokens::operator!=(const Token& other) const;
};

#endif  // QLIB_H
