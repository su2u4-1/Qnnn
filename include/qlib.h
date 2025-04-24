#ifndef QLIB_H
#define QLIB_H

#include <limits.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

class Token;
class Tokens;

extern vector<string> STDLIB;
extern vector<string> BUILTINTYPE;
extern vector<string> OPERATOR;
extern string HELP_DOCS;

bool is_keyword(const string& word);
bool is_symbol(const string& word);
bool is_symbol(char c);
int operator_precedence(const string& op);
bool is_term(const Token& token);
void error(const string& msg, const string& file_name, pair<int, int> pos, const string& source_code);
string get_call_stack();
void add_call_stack(const string& str, const int mode);
void source_code_setitem(string file_name, vector<string> source_code);
string source_code_getitem(string file_name, int line);

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
    Node(const string& type, const map<string, string>& value, const vector<shared_ptr<Node>>& children);
    Node(const string& type, const map<string, string>& value, const shared_ptr<Node>& child);
    Node(const string& type, const map<string, string>& value);
    Node(const string& type);
    Node();
    string type;
    map<string, string> value;
    vector<shared_ptr<Node>> children;
    string toString();
};

#endif  // QLIB_H
