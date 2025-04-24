#include "../include/compiler.h"

int Symbol::symbol_index = 0;

Symbol::Symbol(const string& kind, const string& type, const string& value, int index, bool is_const_or_static)
    : kind(kind), type(type), value(value), index(index), is_const_or_static(is_const_or_static) {
    symbol_index = index;
}

Symbol::Symbol(const string& kind, const string& type, const string& value, int index)
    : kind(kind), type(type), value(value), index(index), is_const_or_static(false) {
    symbol_index = index;
}

Symbol::Symbol(const string& kind, const string& type, const string& value) : kind(kind), type(type), value(value), index(++symbol_index), is_const_or_static(false) {}

Symbol::Symbol() : kind("None"), type(""), value(""), index(++symbol_index), is_const_or_static(false) {}

Compiler::Compiler(const Node& ast) : ast(ast), target_code(vector<string>()), symbol_table(vector<map<string, Symbol>>{map<string, Symbol>()}) {
}

vector<string> Compiler::compile() {
    symbol_table.push_back(map<string, Symbol>());
    symbol_table[0]["__file_name"] = Symbol("env", "string", ast.value["name"], 0);
    target_code.push_back("# start #");
    for (const shared_ptr<Node>& i : ast.children) {
        if (i->type == "class") {
            compile_class(*i);
        } else if (i->type == "import") {
            compile_import(*i);
        } else if (i->type == "declare") {
            compile_declare(*i);
        } else if (i->type == "expression") {
            compile_expression(*i);
        } else if (i->type == "function") {
            compile_function(*i);
        } else if (i->type == "arr") {
            compile_arr(*i);
        } else if (i->type == "tuple") {
            compile_tuple(*i);
        } else if (i->type == "dict") {
            compile_dict(*i);
        } else if (i->type == "if") {
            compile_if(*i);
        } else if (i->type == "for") {
            compile_for(*i);
        } else if (i->type == "while") {
            compile_while(*i);
        }
    }
}

void Compiler::compile_class(const Node& node) {
}

void Compiler::compile_import(const Node& node) {
}

void Compiler::compile_declare(const Node& node) {
}

void Compiler::compile_type(const Node& node) {
}

void Compiler::compile_expression(const Node& node) {
}

void Compiler::compile_term(const Node& node) {
}

void Compiler::compile_variable(const Node& node) {
}

void Compiler::compile_use_typevar(const Node& node) {
}

void Compiler::compile_declare_typevar(const Node& node) {
}

void Compiler::compile_call(const Node& node) {
}

void Compiler::compile_function(const Node& node) {
}

void Compiler::compile_class(const Node& node) {
}

void Compiler::compile_method(const Node& node) {
}

void Compiler::compile_declare_args(const Node& node) {
}

void Compiler::compile_arr(const Node& node) {
}

void Compiler::compile_tuple(const Node& node) {
}

void Compiler::compile_dict(const Node& node) {
}

void Compiler::compile_statements(const Node& node) {
}

void Compiler::compile_statement(const Node& node) {
}

void Compiler::compile_if(const Node& node) {
}

void Compiler::compile_for(const Node& node) {
}

void Compiler::compile_while(const Node& node) {
}

void Compiler::compile_break(const Node& node) {
}

void Compiler::compile_return(const Node& node) {
}
