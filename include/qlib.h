#ifndef QLIB_H
#define QLIB_H

#include <limits.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(_WIN32)
#define PLATFORM_NAME "windows"  // Windows
#elif defined(_WIN64)
#define PLATFORM_NAME "windows"  // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define PLATFORM_NAME "windows"  // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__linux__)
#define PLATFORM_NAME "linux"
#endif

using namespace std;
namespace fs = filesystem;

class Token;
class Tokens;

extern vector<string> STDLIB;
extern vector<string> BUILTINTYPE;
extern vector<string> OPERATOR;
extern string HELP_DOCS;
extern fs::path BASEPATH;

bool is_keyword(const string& word);
bool is_symbol(const string& word);
bool is_symbol(char c);
int operator_precedence(const string& op);
bool is_term(const Token& token);
void error(const string& msg, const fs::path& file_name, pair<int, int> pos, const string& source_code);
string path_processing(const fs::path& file_path);
string get_call_stack();
void add_call_stack(const string& str, const int mode);
void clear_call_stack();
void source_code_setitem(fs::path file_name, vector<string> source_code);
string source_code_getitem(fs::path file_name, int line);

class Token {
   public:
    Token(const string& type, const string& value, const fs::path& file_name, pair<int, int> pos);
    Token(const string& type, const string& value);
    Token();
    string type;
    string value;
    fs::path file_name;
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
