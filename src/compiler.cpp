#include "../include/compiler.h"

Compiler::Compiler(const Node& ast) : ast(ast), target_code(vector<string>()), symbol_table(vector<map<string, string>>{map<string, string>()}) {
}

vector<string> Compiler::compile() {
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
