#ifndef QLIB_H
#define QLIB_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Token {
   public:
    Token(int type, string value, int line, int column);
    int type;
    string value;
    int line;
    int column;
    string toString();
    bool operator==(const Token& other) const;
    bool operator!=(const Token& other) const;
};

class Tokens {
   public:
    Tokens(int type, vector<string> value);
    int type;
    vector<string> value;
    string toString();
};

#endif  // QLIB_H
