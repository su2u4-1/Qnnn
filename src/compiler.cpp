#include "../include/compiler.h"

// Symbol
Symbol::Symbol(const string& kind, const Type& type, const string& value, int index) : kind(kind), type(type), value(value), index(index) {}

Symbol::Symbol() : kind("None"), type(Type()), value(""), index(-1) {}

bool Symbol::operator==(const Symbol& other) {
    return kind == other.kind && type == other.type;
}

string Symbol::toString() const {
    return "Symbol(" + kind + ", " + type.toString() + ", " + value + ", " + to_string(index) + ")";
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

string Type::toString() const {
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
Compiler::Compiler(const Node& ast, Log& log) : ast(ast), log(log) {
    symbol_table.push_back(map<string, Symbol>());
}

void Compiler::compile_error(const string& message, pair<int, int> pos) {
    error("CompileError: " + message, ast.value.at("name"), pos, source_code_getitem(ast.value.at("name"), pos.first - 1));
}

void Compiler::add_subroutine(const string& name, const string& kind, const vector<string>& generic, const vector<Type>& args_type, const Type& return_type) {
    log.log_msg("add_subroutine0", 0);
    string sign = kind + " " + name;
    if (generic.size() > 0) {
        sign += "<";
        for (int i = 0; i < generic.size() - 1; i++)
            sign += generic[i] + ", ";
        sign += generic.back();
        sign += ">";
    }
    sign += "(";
    if (args_type.size() > 0) {
        for (int i = 0; i < args_type.size() - 1; i++)
            sign += args_type[i].toString() + ", ";
        sign += args_type.back().toString();
    }
    sign += ") -> " + return_type.toString();
    target_code.push_back("subroutine \"" + sign + '"');
    if (subroutine_table.find(name) != subroutine_table.end())
        subroutine_table[name] = vector<Symbol>();
    subroutine_table[name].push_back(Symbol("subroutine", Type(kind, {Type(name), Type("generic_number:" + to_string(generic.size())), Type("args", args_type), return_type}), name, subroutine_index++));
    log.log_msg("add_subroutine0", 1);
}

void Compiler::print_info(const string& msg) const {
    cout << "--------------------" << endl;
    cout << msg << endl;
    cout << "subroutine_table: {" << endl;
    for (pair<const string, vector<Symbol>> i : subroutine_table) {
        cout << "    " << i.first << ": [" << endl;
        for (Symbol j : i.second) {
            cout << "        " << j.toString() << "," << endl;
        }
        cout << "    ]," << endl;
    }
    cout << "}" << endl;
    cout << "symbol_table: [" << endl;
    for (map<string, Symbol> i : symbol_table) {
        cout << "    {" << endl;
        for (pair<const string, Symbol> j : i) {
            cout << "        " << j.first << ": " << j.second.toString() << "," << endl;
        }
        cout << "    }," << endl;
    }
    cout << "]" << endl;
    cout << "loop_label_stack: [";
    for (string i : loop_label_stack) {
        cout << i << ", ";
    }
    cout << "]" << endl;
}

vector<string> Compiler::compile() {
    log.log_msg("compile", 0);
    symbol_table[0]["__file_name"] = Symbol("env", Type("string"), ast.value.at("name"), global_index++);
    loop_label_stack = vector<string>();
    target_code.push_back("# start #");
    for (const shared_ptr<Node>& i : ast.children) {
        if (i->type == "class")
            compile_class(*i);
        else if (i->type == "import")
            compile_import(*i);
        else if (i->type == "declare")
            compile_declare_var(*i);
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
    // print_info("end");
    log.log_msg("compile", 1);
    return target_code;
}

void Compiler::compile_import(const Node& node) {
    log.log_msg("compile_import", 0);
    if (node.type != "import")
        compile_error("CompileError: Unknown import type: " + node.type, node.pos);
    fs::path path;
    if (node.value.at("alias") == "stdlib") {
        symbol_table[0][node.value.at("name")] = Symbol("import", Type("stdlib"), node.value.at("name"), global_index++);
        path = path_processing(STDLIBPATH / (node.value.at("name") + ".qn"));
    } else {
        symbol_table[0][node.value.at("alias")] = Symbol("import", Type("userlib"), node.value.at("name"), global_index++);
        path = path_processing(fs::path(ast.value.at("name")).parent_path() / fs::path(node.value.at("name")).replace_extension(".qn"));
    }
    import_list.push_back(path_processing(path));
    log.log_msg("compile_import", 1);
}

void Compiler::compile_declare_var(const Node& node) {
    log.log_msg("compile_declare_var", 0);
    if (node.type != "declare_var")
        compile_error("CompileError: Unknown declare type: " + node.type, node.pos);
    if (node.value.at("modifier") == "global") {
        symbol_table[0][node.value.at("name")] = Symbol(node.value.at("kind"), compile_type(*node.children[0]), node.value.at("name"), global_index++);
    } else
        symbol_table.back()[node.value.at("name")] = Symbol(node.value.at("kind"), compile_type(*node.children[0]), node.value.at("name"), symbol_table.back().size());
    log.log_msg("compile_declare_var", 1);
}

void Compiler::compile_declare_attr(const Node& node, const string& class_name) {
    log.log_msg("compile_declare_attr", 0);
    if (node.type != "declare_attr")
        compile_error("CompileError: Unknown declare type: " + node.type, node.pos);
    if (node.value.at("kind") == "static")
        symbol_table[0][class_name + "::" + node.value.at("name")] = Symbol(node.value.at("modifier"), compile_type(*node.children[0]), class_name + "::" + node.value.at("name"), static_index++);
    else
        symbol_table.back()[class_name + "::" + node.value.at("name")] = Symbol(node.value.at("modifier"), compile_type(*node.children[0]), class_name + "::" + node.value.at("name"), symbol_table.back().size());
    log.log_msg("compile_declare_attr", 1);
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
                target_code.push_back("pop $Y");
                target_code.push_back("pop $X");
                target_code.push_back("add $X $Y $X");
                target_code.push_back("push $X");
            } else if (i->value.at("value") == "-=") {
                target_code.push_back("pop $Y");
                target_code.push_back("pop $X");
                target_code.push_back("sub $X $Y $X");
                target_code.push_back("push $X");
            } else if (i->value.at("value") == "*=") {
                target_code.push_back("pop $Y");
                target_code.push_back("pop $X");
                target_code.push_back("mul $X $Y $X");
                target_code.push_back("push $X");
            } else if (i->value.at("value") == "/=") {
                target_code.push_back("pop $Y");
                target_code.push_back("pop $X");
                target_code.push_back("div $X $Y $X");
                target_code.push_back("push $X");
            } else if (i->value.at("value") == "%=") {
                target_code.push_back("pop $Y");
                target_code.push_back("pop $X");
                target_code.push_back("rem $X $Y $X");
                target_code.push_back("push $X");
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
    target_code.push_back("push term  // " + node.toString());
    // TODO
    log.log_msg("compile_term", 1);
}

void Compiler::compile_variable(const Node& node) {
    log.log_msg("compile_variable", 0);
    if (node.type != "variable")
        compile_error("CompileError: Unknown variable type: " + node.type, node.pos);
    log.log_msg("compile_variable", 1);
}

void Compiler::compile_use_generic(const Node& node) {
    log.log_msg("compile_use_generic", 0);
    if (node.type != "use_generic")
        compile_error("CompileError: Unknown use_generic type: " + node.type, node.pos);
    log.log_msg("compile_use_generic", 1);
}

void Compiler::compile_declare_generic(const Node& node) {
    log.log_msg("compile_declare_generic", 0);
    if (node.type != "declare_generic")
        compile_error("CompileError: Unknown declare_generic type: " + node.type, node.pos);
    log.log_msg("compile_declare_generic", 1);
}

void Compiler::compile_call(const Node& node) {
    log.log_msg("compile_call", 0);
    if (node.type != "call")
        compile_error("CompileError: Unknown call type: " + node.type, node.pos);
    log.log_msg("compile_call", 1);
}

void Compiler::compile_function(const Node& node) {
    log.log_msg("compile_function", 0);
    if (node.type != "func")
        compile_error("CompileError: Unknown function type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    vector<Type> args_type;
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        Type t = compile_type(*i->children[0]);
        if (i->value["tuple"] == "true")
            t = Type("tuple", {t});
        args_type.push_back(t);
        symbol_table.back()[i->value.at("name")] = Symbol("arg", Type(t), i->value.at("name"), symbol_table.back().size());
    }
    vector<string> generics;
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        generics.push_back(i->value.at("name"));
        symbol_table.back()[i->value.at("name")] = Symbol("generic", Type("type"), i->value.at("name"), symbol_table.back().size());
    }
    add_subroutine(node.value.at("name"), node.value.at("const") == "true" ? "const function" : "function", generics, args_type, return_type);
    compile_statements(*node.children[3]);
    // print_info("function");
    symbol_table.pop_back();
    log.log_msg("compile_function", 1);
    return;
}

void Compiler::compile_class(const Node& node) {
    log.log_msg("compile_class", 0);
    if (node.type != "class")
        compile_error("CompileError: Unknown class type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    vector<Type> generic;
    for (const shared_ptr<Node>& i : node.children[0]->children) {
        generic.push_back(Type(i->value.at("name")));
        symbol_table.back()[i->value.at("name")] = Symbol("generic", Type("type"), i->value.at("name"), symbol_table.back().size());
    }
    symbol_table[0][node.value.at("name")] = Symbol("class", Type("class", {Type(node.value.at("name")), Type("generic_number:" + to_string(generic.size()))}), node.value.at("name"), global_index++);
    string sign = "class " + node.value.at("name");
    if (generic.size() > 0) {
        sign += "<";
        for (int i = 0; i < generic.size() - 1; i++)
            sign += generic[i].type + ", ";
        sign += generic.back().type + ">";
    }
    target_code.push_back("# " + sign + " #");
    // print_info("class");
    for (int index = 1; index < node.children.size(); ++index) {
        shared_ptr<Node> i = node.children[index];
        if (i->type == "func")
            compile_function(*i);
        else if (i->type == "method")
            compile_method(*i, node.value.at("name"));
        else if (i->type == "declare_attr")
            compile_declare_attr(*i, node.value.at("name"));
        else
            compile_error("Unknown class member type: " + i->type, i->pos);
    }
    symbol_table.pop_back();
    log.log_msg("compile_class", 1);
}

void Compiler::compile_method(const Node& node, const string& class_name) {
    log.log_msg("compile_method", 0);
    if (node.type != "method")
        compile_error("CompileError: Unknown method type: " + node.type, node.pos);
    symbol_table.push_back(map<string, Symbol>());
    Type return_type = compile_type(*node.children[0]);
    vector<Type> args_type;
    for (const shared_ptr<Node>& i : node.children[2]->children) {
        Type t = compile_type(*i->children[0]);
        if (i->value["tuple"] == "true")
            t = Type("tuple", {t});
        args_type.push_back(t);
        symbol_table.back()[i->value.at("name")] = Symbol("arg", Type(t), i->value.at("name"), symbol_table.back().size());
    }
    vector<string> generics;
    for (const shared_ptr<Node>& i : node.children[1]->children) {
        generics.push_back(i->value.at("name"));
        symbol_table.back()[i->value.at("name")] = Symbol("generic", Type("type"), i->value.at("name"), symbol_table.back().size());
    }
    add_subroutine(class_name + "::" + node.value.at("name"), node.value.at("kind") != "private" ? node.value.at("kind") + " method" : "method", generics, args_type, return_type);
    compile_statements(*node.children[3]);
    // print_info("method");
    symbol_table.pop_back();
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
            compile_declare_var(*i);
        else if (i->type == "declare_generic")
            compile_declare_generic(*i);
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
    target_code.push_back("goto " + loop_label_stack.back() + "_start");
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
    string n = "";
    if (node.value.at("label") == "for")
        n += "loop_" + to_string(loop_index++);
    else {
        symbol_table.back()[node.value.at("label")] = Symbol("loop_label", Type("string"), node.value.at("label"), symbol_table.back().size());
        n += node.value.at("label");
        loop_index++;
    }
    target_code.push_back("label \"" + n + "_start\"");
    loop_label_stack.push_back(n);
    compile_statements(*node.children[2]);
    print_info("for");
    target_code.push_back("label \"" + n + "_end\"");
    loop_label_stack.pop_back();
    log.log_msg("compile_for", 1);
}

void Compiler::compile_while(const Node& node) {
    log.log_msg("compile_while", 0);
    if (node.type != "while")
        compile_error("CompileError: Unknown while type: " + node.type, node.pos);
    string n = "";
    if (node.value.at("label") == "while")
        n += "loop_" + to_string(loop_index++);
    else {
        symbol_table.back()[node.value.at("label")] = Symbol("loop_label", Type("string"), node.value.at("label"), symbol_table.back().size());
        n += node.value.at("label");
        loop_index++;
    }
    target_code.push_back("label \"" + n + "_start\"");
    loop_label_stack.push_back(n);
    compile_statements(*node.children[1]);
    compile_expression(*node.children[0]);
    target_code.push_back("pop $X");
    target_code.push_back("if_goto $X " + n + "_start");
    print_info("while");
    target_code.push_back("label \"" + n + "_end\"");
    loop_label_stack.pop_back();
    log.log_msg("compile_while", 1);
}

void Compiler::compile_break(const Node& node) {
    log.log_msg("compile_break", 0);
    if (node.type != "break")
        compile_error("CompileError: Unknown break type: " + node.type, node.pos);
    if (loop_label_stack.empty())
        compile_error("CompileError: Break statement not in loop", node.pos);
    if (node.value.at("label") != "break") {
        bool found = false;
        while (!loop_label_stack.empty()) {
            if (loop_label_stack.back() == node.value.at("label")) {
                target_code.push_back("goto " + loop_label_stack.back() + "_end");
                loop_label_stack.pop_back();
                found = true;
                break;
            }
            loop_label_stack.pop_back();
        }
        if (!found)
            compile_error("CompileError: Break label not found: " + node.value.at("label"), node.pos);
    } else {
        target_code.push_back("goto " + loop_label_stack.back() + "_end");
        loop_label_stack.pop_back();
    }
    print_info("break");
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
