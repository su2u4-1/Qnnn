#ifndef COMPILER_H
#define COMPILER_H

#include "qlib.h"

struct Type {
    string type;
    vector<Type> args;
    Type(const string& type, const vector<Type>& args);
    Type(const string& type);
    Type();
    bool operator==(const Type& other);
    bool operator!=(const Type& other);
    string toString() const;
};

struct Symbol {
    string kind;
    Type type;
    string value;
    int index;
    Symbol(const string& kind, const Type& type, const string& value, int index);
    Symbol();
    bool operator==(const Symbol& other);
    string toString() const;
};

class Compiler {
   public:
    Compiler(const Node& ast, Log& log);
    Node ast;
    Log log;
    vector<string> target_code;
    vector<map<string, Symbol>> symbol_table;
    map<string, vector<Symbol>> subroutine_table;
    vector<fs::path> import_list;
    vector<string> loop_label_stack;
    int static_index;
    int global_index;
    int subroutine_index;
    void add_subroutine(const string& name, const string& kind, const vector<string>& generic, const vector<Type>& args_type, const Type& return_type);
    void print_info(const string& msg) const;
    vector<string> compile();
    void compile_error(const string& message, pair<int, int> pos);
    void compile_class(const Node& node);
    void compile_import(const Node& node);
    void compile_declare_var(const Node& node);
    void compile_declare_attr(const Node& node, const string& class_name);
    const Type compile_type(const Node& node);
    void compile_expression(const Node& node);
    void compile_term(const Node& node);
    void compile_variable(const Node& node);
    void compile_use_generic(const Node& node);
    void compile_declare_generic(const Node& node);
    void compile_call(const Node& node);
    void compile_function(const Node& node);
    void compile_method(const Node& node, const string& class_name);
    void compile_arr(const Node& node);
    void compile_tuple(const Node& node);
    void compile_dict(const Node& node);
    void compile_statements(const Node& node);
    void compile_continue(const Node& node);
    void compile_if(const Node& node);
    void compile_for(const Node& node);
    void compile_while(const Node& node);
    void compile_break(const Node& node);
    void compile_return(const Node& node);
};

#endif  // COMPILER_H
