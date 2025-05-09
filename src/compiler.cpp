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

// Compiler
Compiler::Compiler(const Node& ast) : ast(ast), target_code(vector<string>()), symbol_table(vector<map<string, Symbol>>{map<string, Symbol>()}), import_list(vector<fs::path>()) {}

void Compiler::compile_error(const string& message, pair<int, int> pos) {
    error("CompileError: " + message, ast.value["name"], pos, source_code_getitem(ast.value["name"], pos.first - 1));
}

vector<string> Compiler::compile() {
    symbol_table.push_back(map<string, Symbol>());
    symbol_table[0]["__file_name"] = Symbol("env", Type("string"), ast.value["name"], 0);
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
        else if (i->type == "function")
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
    return target_code;
}

void Compiler::compile_import(const Node& node) {
    fs::path path;
    if (node.value.at("alias") == "stdlib") {
        symbol_table.back()[node.value.at("name")] = Symbol("import", Type("stdlib"), node.value.at("name"));
        path = BASEPATH / ("stdlib/" + node.value.at("name") + ".qn");
    } else {
        symbol_table.back()[node.value.at("alias")] = Symbol("import", Type("userlib"), node.value.at("name"));
        path = fs::path(ast.value["name"]).parent_path() / fs::path(node.value.at("name")).replace_extension(".qn");
    }
    import_list.push_back(path_processing(path));
}

void Compiler::compile_declare(const Node& node) {
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
}

const Type& Compiler::compile_type(const Node& node) {
    vector<Type> args;
    for (const shared_ptr<Node>& i : node.children) {
        args.push_back(compile_type(*i));
    }
    return Type(node.value.at("name"), args);
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

// function_type = (<isConst>(bool), <return_type>(type), [<arg_type>(type), ...])
// symbol_table[0][function_name] = function_type
// symbol_table[-1] = {<arg_name>: Symbol("arg", arg_type, <arg_name>), ..., <typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
void Compiler::compile_function(const Node& node) {
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    Type args_type("args");
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        const Type& t = compile_type(*i);
        args_type.args.push_back(t);
        symbol_table.back()[i->value["name"]] = Symbol("arg", Type(t), i->value["name"]);
    }
    symbol_table[0][node.value.at("name")] = Symbol("function", Type("function", {Type(node.value.at("const")), return_type, args_type}), node.value.at("name"));
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        symbol_table.back()[i->value["name"]] = Symbol("typevar", Type("type"), i->value["name"]);
    }
    compile_statements(*node.children[3]);
    return;
}

// class_type = ([<arg_type>(type), ...])
// symbol_table[-1] = {<typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
// symbol_table[0][function_name] = class_type
void Compiler::compile_class(const Node& node) {
    symbol_table.push_back(map<string, Symbol>());
    Type args_type("args");
    for (const shared_ptr<Node>& i : node.children[0]->children) {
        args_type.args.push_back(Type(i->value["name"]));
        symbol_table.back()[i->value["name"]] = Symbol("typevar", Type("type"), i->value["name"]);
    }
    symbol_table[0][node.value.at("name")] = Symbol("class", Type("class", {args_type}), node.value.at("name"));
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
    compile_statements(*node.children[3]);
}

// method_type = (<kind>(str), <return_type>(type), [<arg_type>(type), ...])
// symbol_table[-2][method_name] = method_type
// symbol_table[-1] = {<arg_name>: Symbol("arg", arg_type, <arg_name>), ..., <typevar_name>: Symbol("typevar", Type("type"), <typevar_name>), ...}
void Compiler::compile_method(const Node& node) {
    map<string, Symbol>& class_symbol_table = symbol_table.back();
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    Type args_type("args");
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        const Type& t = compile_type(*i);
        args_type.args.push_back(t);
        symbol_table.back()[i->value["name"]] = Symbol("arg", Type(t), i->value["name"]);
    }
    class_symbol_table[node.value.at("name")] = Symbol("method", Type("method", {Type(node.value.at("kind")), return_type, args_type}), node.value.at("name"));
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        symbol_table.back()[i->value["name"]] = Symbol("typevar", Type("type"), i->value["name"]);
    }
    compile_statements(*node.children[3]);
    return;
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
