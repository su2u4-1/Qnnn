#ifndef COMPILER_H
#define COMPILER_H

#include "qlib.h"

int symbol_index;

struct Symbol {
    string kind;
    Type type;
    string value;
    int index;
    static int symbol_index;
    Symbol(const string& kind, const Type& type, const string& value, int index);
    Symbol(const string& kind, const Type& type, const string& value);
    Symbol();
};
struct Type {
    string type;
    vector<string> args;
    Type(const string& type, const vector<string>& args);
    Type(const string& type);
    Type();
};

class Compiler {
   public:
    Compiler(const Node& ast);
    Node ast;
    vector<string> target_code;
    vector<map<string, Symbol>> symbol_table;
    vector<string> compile();
    void compile_class(const Node& node);
    void compile_import(const Node& node);
    void compile_declare(const Node& node);
    Type compile_type(const Node& node);
    void compile_expression(const Node& node);
    void compile_term(const Node& node);
    void compile_variable(const Node& node);
    void compile_use_typevar(const Node& node);
    void compile_declare_typevar(const Node& node);
    void compile_call(const Node& node);
    void compile_function(const Node& node);
    void compile_method(const Node& node);  // 移除重複的 compile_class 宣告
    void compile_declare_args(const Node& node);
    void compile_arr(const Node& node);
    void compile_tuple(const Node& node);
    void compile_dict(const Node& node);
    void compile_statements(const Node& node);
    void compile_statement(const Node& node);
    void compile_if(const Node& node);
    void compile_for(const Node& node);
    void compile_while(const Node& node);
    void compile_break(const Node& node);
    void compile_return(const Node& node);
};

#endif  // COMPILER_H
