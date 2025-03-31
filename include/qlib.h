#ifndef QLIB_H
#define QLIB_H

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

extern vector<string> STDLIB;
extern vector<string> BUILTINTYPE;

bool is_keyword(const string& word);
bool is_symbol(const string& word);
bool is_symbol(char c);
bool is_term(const Token& token);
void error(const string& msg, const string& file_name, pair<int, int> pos, const string& source_code);
void source_code_setitem(string file_name, vector<string> source_code);
string source_code_getitem(string file_name, int line);

class Tokens;

class Token {
   public:
    Token(const string& type, const string& value, const string& file_name, pair<int, int> pos);
    Token(const string& type, const string& value);
    Token();
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
    Tokens(const string& type, const vector<string>& value);
    string type;
    vector<string> value;
    string toString();
    bool operator==(const Token& other) const;
    bool operator!=(const Token& other) const;
};

class Node {
   public:
    Node(const string& type, const map<string, string>& value, const vector<Node>& children);
    Node(const string& type, const map<string, string>& value, const Node& child);
    Node(const string& type, const map<string, string>& value);
    Node(const string& type);
    Node();
    string type;
    map<string, string> value;
    vector<Node> children;
    string toString();
};

#endif  // QLIB_H
