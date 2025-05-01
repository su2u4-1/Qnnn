#include "../include/compiler.h"

// Symbol
int Symbol::symbol_index = 0;

Symbol::Symbol(const string& kind, const Type& type, const string& value, int index)
    : kind(kind), type(type), value(value), index(index) {
    symbol_index = index;
}

Symbol::Symbol(const string& kind, const Type& type, const string& value) : kind(kind), type(type), value(value), index(++symbol_index) {}

Symbol::Symbol() : kind("None"), type(Type()), value(""), index(++symbol_index) {}

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

vector<string> Compiler::compile() {
    symbol_table.push_back(map<string, Symbol>());
    symbol_table[0]["__file_name"] = Symbol("env", Type("string"), ast.value["name"], 0);
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

void Compiler::compile_class(const Node& node) {
    symbol_table.push_back(map<string, Symbol>());
}

void Compiler::compile_method(const Node& node) {
    symbol_table.push_back(map<string, Symbol>());
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
