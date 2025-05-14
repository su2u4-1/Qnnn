#include "../include/compiler.h"

// Symbol
int Symbol::symbol_index = 0;

Symbol::Symbol(const string& kind, const Type& type, const string& value, int index)
    : kind(kind), type(type), value(value), index(index) {
    symbol_index = index;
}

Symbol::Symbol(const string& kind, const Type& type, const string& value) : kind(kind), type(type), value(value), index(++symbol_index) {}

Symbol::Symbol() : kind("None"), type(Type()), value(""), index(++symbol_index) {}

bool Symbol::operator==(const Symbol& other) {
    return kind == other.kind && type == other.type;
}

// Type
Type::Type(const string& type, const vector<Type>& args) : type(type), args(args) {}

Type::Type(const string& type) : type(type), args(vector<Type>()) {}

Type::Type() : type("None"), args(vector<Type>()) {}

bool Type::operator==(const Type& other) {
    if (type != other.type)
        return false;
    if (args.size() != other.args.size())
        return false;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] != other.args[i])
            return false;
    }
    return true;
}

bool Type::operator!=(const Type& other) {
    return !(*this == other);
}

string Type::toString() {
    string t = type;
    if (args.size() > 0) {
        t += "<";
        for (int i = 0; i < args.size() - 1; i++)
            t += args[i].toString() + ", ";
        t += args.back().toString();
        t += ">";
    }
    return t;
}

// Compiler
Compiler::Compiler(const Node& ast, Log& log) : ast(ast), target_code(vector<string>()), symbol_table(vector<map<string, Symbol>>{map<string, Symbol>()}), import_list(vector<fs::path>()), log(log) {}

void Compiler::compile_error(const string& message, pair<int, int> pos) {
    error("CompileError: " + message, ast.value.at("name"), pos, source_code_getitem(ast.value.at("name"), pos.first - 1));
}

vector<string> Compiler::compile() {
    log.log_msg("compile", 0);
    symbol_table.push_back(map<string, Symbol>());
    symbol_table[0]["__file_name"] = Symbol("env", Type("string"), ast.value.at("name"), 0);
    loop_label_stack = vector<string>();
    target_code.push_back("# start #");
    for (const shared_ptr<Node>& i : ast.children) {
        if (i->type == "class")
            compile_class(*i);
        else if (i->type == "import")
            compile_import(*i);
        else if (i->type == "declare")
            compile_declare(*i);
        else if (i->type == "expression")
            compile_expression(*i);
        else if (i->type == "func")
            compile_function(*i);
        else if (i->type == "arr")
            compile_arr(*i);
        else if (i->type == "tuple")
            compile_tuple(*i);
        else if (i->type == "dict")
            compile_dict(*i);
        else if (i->type == "if")
            compile_if(*i);
        else if (i->type == "for")
            compile_for(*i);
        else if (i->type == "while")
            compile_while(*i);
    }
    log.log_msg("compile", 1);
    return target_code;
}

void Compiler::compile_import(const Node& node) {
    log.log_msg("compile_import", 0);
    if (node.type != "import")
        compile_error("CompileError: Unknown import type: " + node.type, node.pos);
    fs::path path;
    if (node.value.at("alias") == "stdlib") {
        symbol_table.back()[node.value.at("name")] = Symbol("import", Type("stdlib"), node.value.at("name"));
        path = path_processing(STDLIBPATH / (node.value.at("name") + ".qn"));
    } else {
        symbol_table.back()[node.value.at("alias")] = Symbol("import", Type("userlib"), node.value.at("name"));
        path = path_processing(fs::path(ast.value.at("name")).parent_path() / fs::path(node.value.at("name")).replace_extension(".qn"));
    }
    import_list.push_back(path_processing(path));
    log.log_msg("compile_import", 1);
}

void Compiler::compile_declare(const Node& node) {
    log.log_msg("compile_declare", 0);
    if (node.type != "declare_var" && node.type != "declare_attr")
        compile_error("CompileError: Unknown declare type: " + node.type, node.pos);
    if (node.type == "declare_var") {
        if (node.value.at("modifier") == "global")
            symbol_table[0][node.value.at("name")] = Symbol(node.value.at("kind"), compile_type(*node.children[0]), node.value.at("name"));
        else
            symbol_table.back()[node.value.at("name")] = Symbol(node.value.at("kind"), compile_type(*node.children[0]), node.value.at("name"));
    } else if (node.type == "declare_attr") {
        if (node.value.at("kind") == "static")
            symbol_table[symbol_table.size() - 2][node.value.at("name")] = Symbol(node.value.at("modifier"), compile_type(*node.children[0]), node.value.at("name"));
        else
            symbol_table.back()[node.value.at("name")] = Symbol(node.value.at("modifier"), compile_type(*node.children[0]), node.value.at("name"));
    }
    log.log_msg("compile_declare", 1);
}

const Type Compiler::compile_type(const Node& node) {
    log.log_msg("compile_type", 0);
    if (node.type != "type")
        compile_error("CompileError: Unknown type type: " + node.type, node.pos);
    vector<Type> args;
    for (const shared_ptr<Node>& i : node.children) {
        args.push_back(compile_type(*i));
    }
    Type t(node.value.at("name"), args);
    log.log_msg("compile_type", 1);
    return t;
}

void Compiler::compile_expression(const Node& node) {
    log.log_msg("compile_expression", 0);
    if (node.type != "expression")
        compile_error("CompileError: Unknown expression type: " + node.type, node.pos);
    for (const shared_ptr<Node>& i : node.children) {
        if (i->type == "operator") {
            if (i->value.at("value") == "+")
                target_code.push_back("add");
            else if (i->value.at("value") == "-")
                target_code.push_back("sub");
            else if (i->value.at("value") == "*")
                target_code.push_back("mul");
            else if (i->value.at("value") == "/")
                target_code.push_back("div");
            else if (i->value.at("value") == "%")
                target_code.push_back("rem");
            else if (i->value.at("value") == "==")
                target_code.push_back("eq");
            else if (i->value.at("value") == "!=")
                target_code.push_back("neq");
            else if (i->value.at("value") == "<")
                target_code.push_back("lt");
            else if (i->value.at("value") == ">")
                target_code.push_back("gt");
            else if (i->value.at("value") == "<=")
                target_code.push_back("leq");
            else if (i->value.at("value") == ">=")
                target_code.push_back("geq");
            else if (i->value.at("value") == "&&")
                target_code.push_back("land");
            else if (i->value.at("value") == "||")
                target_code.push_back("lor");
            else if (i->value.at("value") == "&")
                target_code.push_back("band");
            else if (i->value.at("value") == "|")
                target_code.push_back("bor");
            else if (i->value.at("value") == "=")
                target_code.push_back("asi");
            else if (i->value.at("value") == "+=") {
                target_code.push_back("pop $t1");
                target_code.push_back("pop $t0");
                target_code.push_back("add $t0 $t1 $t0");
                target_code.push_back("push $t0");
            } else if (i->value.at("value") == "-=") {
                target_code.push_back("pop $t1");
                target_code.push_back("pop $t0");
                target_code.push_back("sub $t0 $t1 $t0");
                target_code.push_back("push $t0");
            } else if (i->value.at("value") == "*=") {
                target_code.push_back("pop $t1");
                target_code.push_back("pop $t0");
                target_code.push_back("mul $t0 $t1 $t0");
                target_code.push_back("push $t0");
            } else if (i->value.at("value") == "/=") {
                target_code.push_back("pop $t1");
                target_code.push_back("pop $t0");
                target_code.push_back("div $t0 $t1 $t0");
                target_code.push_back("push $t0");
            } else if (i->value.at("value") == "%=") {
                target_code.push_back("pop $t1");
                target_code.push_back("pop $t0");
                target_code.push_back("rem $t0 $t1 $t0");
                target_code.push_back("push $t0");
            } else if (i->value.at("value") == "**")
                target_code.push_back("pow");
            else if (i->value.at("value") == "@")
                target_code.push_back("ptr");
            else if (i->value.at("value") == "^")
                target_code.push_back("def");
            else if (i->value.at("value") == "<<")
                target_code.push_back("lsh");
            else if (i->value.at("value") == ">>")
                target_code.push_back("rsh");
            else if (i->value.at("value") == "!")
                target_code.push_back("not");
        } else if (i->type == "term") {
            compile_term(*i);
        }
    }
    log.log_msg("compile_expression", 1);
}

void Compiler::compile_term(const Node& node) {
    log.log_msg("compile_term", 0);
    if (node.type != "term")
        compile_error("CompileError: Unknown term type: " + node.type, node.pos);
    if (node.value.at("type") == "int" || node.value.at("type") == "bool")
        target_code.push_back("push " + node.value.at("value"));
    else if (node.value.at("type") == "char")
        target_code.push_back("push " + to_string(static_cast<int>(node.value.at("value")[0])));
    else if (node.value.at("type") == "float") {
        size_t pos = node.value.at("value").find('.');
        if (pos == std::string::npos) {
            compile_error("CompileError: Invalid float literal", node.pos);
        }
        target_code.push_back("push " + node.value.at("value").substr(0, pos));
        target_code.push_back("push " + node.value.at("value").substr(pos + 1));
    }
    // TODO
    log.log_msg("compile_term", 1);
}

void Compiler::compile_variable(const Node& node) {
    log.log_msg("compile_variable", 0);
    if (node.type != "variable")
        compile_error("CompileError: Unknown variable type: " + node.type, node.pos);
    log.log_msg("compile_variable", 1);
}

void Compiler::compile_use_typevar(const Node& node) {
    log.log_msg("compile_use_typevar", 0);
    if (node.type != "use_typevar")
        compile_error("CompileError: Unknown use_typevar type: " + node.type, node.pos);
    log.log_msg("compile_use_typevar", 1);
}

void Compiler::compile_declare_typevar(const Node& node) {
    log.log_msg("compile_declare_typevar", 0);
    if (node.type != "declare_typevar")
        compile_error("CompileError: Unknown declare_typevar type: " + node.type, node.pos);
    log.log_msg("compile_declare_typevar", 1);
}

void Compiler::compile_call(const Node& node) {
    log.log_msg("compile_call", 0);
    if (node.type != "call")
        compile_error("CompileError: Unknown call type: " + node.type, node.pos);
    log.log_msg("compile_call", 1);
}

// function_type = (<isConst>(bool), <return_type>(type), [<arg_type>(type), ...])
// symbol_table[0][function_name] = function_type
// symbol_table[-1] = {<arg_name>: Symbol("arg", arg_type, <arg_name>), ..., <typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
void Compiler::compile_function(const Node& node) {
    log.log_msg("compile_function", 0);
    if (node.type != "func")
        compile_error("CompileError: Unknown function type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    Type args_type("args");
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        Type t = compile_type(*i->children[0]);
        if (i->value["tuple"] == "true")
            t = Type("tuple", {t});
        args_type.args.push_back(t);
        symbol_table.back()[i->value.at("name")] = Symbol("arg", Type(t), i->value.at("name"));
    }
    symbol_table[0][node.value.at("name")] = Symbol("function", Type("function", {Type(node.value.at("const")), return_type, args_type}), node.value.at("name"));
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        symbol_table.back()[i->value.at("name")] = Symbol("typevar", Type("type"), i->value.at("name"));
    }
    string function_type = "";
    if (node.value.at("const") == "true")
        function_type = "const_function:" + node.value.at("name") + "(";
    else
        function_type = "function:" + node.value.at("name") + "(";
    if (args_type.args.size() > 0) {
        for (int i = 0; i < args_type.args.size() - 1; i++)
            function_type += args_type.args[i].toString() + ",";
        function_type += args_type.args.back().toString();
    }
    function_type += ")->" + return_type.toString();
    target_code.push_back("subroutine " + function_type);
    compile_statements(*node.children[3]);
    log.log_msg("compile_function", 1);
    return;
}

// class_type = ([<arg_type>(type), ...])
// symbol_table[-1] = {<typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
// symbol_table[0][function_name] = class_type
void Compiler::compile_class(const Node& node) {
    log.log_msg("compile_class", 0);
    if (node.type != "class")
        compile_error("CompileError: Unknown class type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    Type typearg("args");
    for (const shared_ptr<Node>& i : node.children[0]->children) {
        typearg.args.push_back(Type(i->value.at("name")));
        symbol_table.back()[i->value.at("name")] = Symbol("typevar", Type("type"), i->value.at("name"));
    }
    symbol_table[0][node.value.at("name")] = Symbol("class", Type("class", {typearg}), node.value.at("name"));
    target_code.push_back("# " + symbol_table[0][node.value.at("name")].type.toString() + " #");
    for (int index = 1; index < node.children.size(); ++index) {
        shared_ptr<Node> i = node.children[index];
        if (i->type == "func")
            compile_function(*i);
        else if (i->type == "method")
            compile_method(*i);
        else if (i->type == "declare_attr")
            compile_declare(*i);
        else
            compile_error("Unknown class member type: " + i->type, i->pos);
    }
    log.log_msg("compile_class", 1);
}

// method_type = (<kind>(str), <return_type>(type), [<arg_type>(type), ...])
// symbol_table[-2][method_name] = method_type
// symbol_table[-1] = {<arg_name>: Symbol("arg", arg_type, <arg_name>), ..., <typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
void Compiler::compile_method(const Node& node) {
    log.log_msg("compile_method", 0);
    if (node.type != "method")
        compile_error("CompileError: Unknown method type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    Type args_type("args");
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        Type t = compile_type(*i->children[0]);
        if (i->value["tuple"] == "true") {
            t = Type("tuple", {t});
        }
        args_type.args.push_back(t);
        symbol_table.back()[i->value.at("name")] = Symbol("arg", Type(t), i->value.at("name"));
    }
    symbol_table.end()[-2][node.value.at("name")] = Symbol("method", Type("method", {Type(node.value.at("kind")), return_type, args_type}), node.value.at("name"));
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        symbol_table.back()[i->value.at("name")] = Symbol("typevar", Type("type"), i->value.at("name"));
    }
    string method_type = node.value.at("kind") + "_method:" + node.value.at("name") + "(";
    if (args_type.args.size() > 0) {
        for (int i = 0; i < args_type.args.size() - 1; i++)
            method_type += args_type.args[i].toString() + ",";
        method_type += args_type.args.back().toString();
    }
    method_type += ")->" + return_type.toString();
    target_code.push_back("subroutine " + method_type);
    compile_statements(*node.children[3]);
    log.log_msg("compile_method", 1);
    return;
}

void Compiler::compile_arr(const Node& node) {
    log.log_msg("compile_arr", 0);
    if (node.type != "arr")
        compile_error("CompileError: Unknown arr type: " + node.type, node.pos);
    log.log_msg("compile_arr", 1);
}

void Compiler::compile_tuple(const Node& node) {
    log.log_msg("compile_tuple", 0);
    if (node.type != "tuple")
        compile_error("CompileError: Unknown tuple type: " + node.type, node.pos);
    log.log_msg("compile_tuple", 1);
}

void Compiler::compile_dict(const Node& node) {
    log.log_msg("compile_dict", 0);
    if (node.type != "dict")
        compile_error("CompileError: Unknown dict type: " + node.type, node.pos);
    log.log_msg("compile_dict", 1);
}

void Compiler::compile_statements(const Node& node) {
    log.log_msg("compile_statements", 0);
    if (node.type != "statements")
        compile_error("CompileError: Unknown statements type: " + node.type, node.pos);
    for (const shared_ptr<Node>& i : node.children) {
        if (i->type == "declare_var" || i->type == "declare_attr")
            compile_declare(*i);
        else if (i->type == "declare_typevar")
            compile_declare_typevar(*i);
        else if (i->type == "if")
            compile_if(*i);
        else if (i->type == "for")
            compile_for(*i);
        else if (i->type == "while")
            compile_while(*i);
        else if (i->type == "break")
            compile_break(*i);
        else if (i->type == "return")
            compile_return(*i);
        else if (i->type == "continue")
            compile_continue(*i);
        else if (i->type == "func")
            compile_function(*i);
        else if (i->type == "class")
            compile_class(*i);
        else if (i->type == "expression")
            compile_expression(*i);
    }
    log.log_msg("compile_statements", 1);
}

void Compiler::compile_continue(const Node& node) {
    log.log_msg("compile_continue", 0);
    if (node.type != "continue")
        compile_error("CompileError: Unknown continue type: " + node.type, node.pos);
    log.log_msg("compile_continue", 1);
}

void Compiler::compile_if(const Node& node) {
    log.log_msg("compile_if", 0);
    if (node.type != "if")
        compile_error("CompileError: Unknown if type: " + node.type, node.pos);
    log.log_msg("compile_if", 1);
}

void Compiler::compile_for(const Node& node) {
    log.log_msg("compile_for", 0);
    if (node.type != "for")
        compile_error("CompileError: Unknown for type: " + node.type, node.pos);
    log.log_msg("compile_for", 1);
}

void Compiler::compile_while(const Node& node) {
    log.log_msg("compile_while", 0);
    if (node.type != "while")
        compile_error("CompileError: Unknown while type: " + node.type, node.pos);
    log.log_msg("compile_while", 1);
}

void Compiler::compile_break(const Node& node) {
    log.log_msg("compile_break", 0);
    if (node.type != "break")
        compile_error("CompileError: Unknown break type: " + node.type, node.pos);
    log.log_msg("compile_break", 1);
}

void Compiler::compile_return(const Node& node) {
    log.log_msg("compile_return", 0);
    if (node.type != "return")
        compile_error("CompileError: Unknown return type: " + node.type, node.pos);
    if (node.children.size() == 1)
        compile_expression(*node.children[0]);
    else
        target_code.push_back("push 0");
    target_code.push_back("return");
    log.log_msg("compile_return", 1);
}
