#include "../include/parser.h"

#include "../include/qlib.h"

Parser::Parser(const vector<Token>& tokens, const string& file_name) {
    if (!tokens.empty()) {
        this->tokens = tokens;
        this->file_name = file_name;
        this->index = 0;
        this->current_token = Token();
        get_token();
    } else {
        cout << "Error: No tokens provided." << endl;
        exit(1);
    }
}

Token Parser::next_token() {
    if (index < tokens.size()) {
        Token t = tokens[index];
        int n = 0;
        while (t.type == "comment") {
            n++;
            if (index + n >= tokens.size())
                return Token("EOF", "", file_name, {-1, -1});
            t = tokens[index + n];
        }
        return t;
    } else
        return Token("EOF", "", file_name, {-1, -1});
}

void Parser::get_token() {
    if (index < tokens.size()) {
        current_token = tokens[index];
        index++;
        if (current_token.type == "comment") {
            get_token();
        }
    } else
        current_token = Token("EOF", "", file_name, {-1, -1});
    add_call_stack(current_token.toString(), 2);
}

void Parser::parser_error(const string& msg) {
    parser_error(msg, current_token);
}

void Parser::parser_error(const string& msg, const Token& token) {
    error(msg, file_name, {token.line, token.column}, source_code_getitem(file_name, token.line - 1));
}

Node Parser::parse() {
    add_call_stack("parse", 0);
    Node root("program");
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import"))
            root.children.push_back(parse_import());
        else if (current_token == Token("keyword", "class"))
            root.children.push_back(parse_class());
        else if (current_token == Token("keyword", "func"))
            root.children.push_back(parse_function());
        else
            for (const Node& i : parse_statement()) root.children.push_back(i);
        get_token();
    }
    add_call_stack("parse", 1);
    return root;
}

Node Parser::parse_import() {
    add_call_stack("parse_import", 0);
    get_token();
    Node import("import");
    if (current_token == Tokens("identifier", STDLIB)) {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
        import.value["alias"] = "stdlib";
        add_call_stack("parse_import", 1);
        return import;
    } else if (current_token.type == "str") {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("keyword", "as"))
            parser_error("Expected keyword 'as', not " + current_token.toString());
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        import.value["alias"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
        add_call_stack("parse_import", 1);
        return import;
    } else
        parser_error("Expected stdlib or string, not " + current_token.toString());
    add_call_stack("parse_import", 3);
    return Node();
}

vector<Node> Parser::parse_declare(bool attr) {
    add_call_stack("parse_declare", 0);
    map<string, string> state;
    vector<Node> nodes;
    if (attr) {
        if (current_token == Token("keyword", "attr"))
            state["kind"] = "attr";
        else if (current_token == Token("keyword", "static"))
            state["kind"] = "static";
        else
            parser_error("Expected 'attr' or 'static', not " + current_token.toString());
        get_token();
        if (current_token == Token("keyword", "public")) {
            state["modifier"] = "public";
            get_token();
        } else
            state["modifier"] = "local";
    } else {
        if (current_token == Token("keyword", "var"))
            state["kind"] = "var";
        else if (current_token == Token("keyword", "const"))
            state["kind"] = "const";
        else
            parser_error("Expected 'var' or 'const', not " + current_token.toString());
        get_token();
        if (current_token == Token("keyword", "global")) {
            state["modifier"] = "global";
            get_token();
        } else
            state["modifier"] = "local";
    }
    Node type = parse_type();
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    state["name"] = current_token.value;
    get_token();
    Node expression("expression");
    if (current_token == Token("symbol", "=")) {
        get_token();
        expression = parse_expression();
    }
    if (attr)
        nodes.push_back(Node("declare_attr", state, {type, expression}));
    else
        nodes.push_back(Node("declare_var", state, {type, expression}));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        state["name"] = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        if (attr)
            nodes.push_back(Node("declare_attr", state, {type, expression}));
        else
            nodes.push_back(Node("declare_var", state, {type, expression}));
    }
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';', not " + current_token.toString());
    add_call_stack("parse_declare", 1);
    return nodes;
}

Node Parser::parse_type() {
    add_call_stack("parse_type", 0);
    Node type("type");
    if (current_token == Tokens("keyword", BUILTINTYPE) || current_token.type == "identifier")
        type.value["name"] = current_token.value;
    else
        parser_error("Expected built-in type or identifier, not " + current_token.toString());
    if (next_token() == Token("symbol", "<")) {
        get_token();
        do {
            get_token();
            type.children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
    }
    add_call_stack("parse_type", 1);
    return type;
}

Node Parser::parse_expression() {
    add_call_stack("parse_expression", 0);
    Node expression("expression");
    vector<Node> terms_operators;
    terms_operators.push_back(parse_term());
    while (true) {
        if (current_token != Tokens("symbol", OPERATOR))
            break;
        terms_operators.push_back(Node("operator", {{"value", current_token.value}}));
        get_token();
        terms_operators.push_back(parse_term());
    }
    vector<Node> stack;
    for (const Node& i : terms_operators) {
        if (i.type == "term")
            expression.children.push_back(i);
        else if (i.type == "operator") {
            while (!stack.empty() && operator_precedence(stack.back().value.at("value")) >= operator_precedence(i.value.at("value"))) {
                expression.children.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(i);
        }
    }
    while (!stack.empty()) {
        expression.children.push_back(stack.back());
        stack.pop_back();
    }
    add_call_stack("parse_expression", 1);
    return expression;
}

Node Parser::parse_term() {
    add_call_stack("parse_term", 0);
    Node term("term");
    if (is_term(current_token)) {
        if (current_token == Tokens("symbol", {"^", "@", "-", "!"})) {  // type = one operator
            if (current_token.value == "^") {
                term.value["type"] = "dereference";
            } else if (current_token.value == "@") {
                term.value["type"] = "pointer";
            } else if (current_token.value == "-") {
                term.value["type"] = "neg";
            } else if (current_token.value == "!") {
                term.value["type"] = "not";
            }
            get_token();
            term.children.push_back(parse_term());
        } else if (current_token == Token("symbol", "(")) {  // type = value or expression or tuple
            term.children.push_back(parse_tuple());
            term.value["type"] = term.children.back().type;
            get_token();
        } else if (current_token == Token("symbol", "[")) {  // type = arr
            term.children.push_back(parse_arr());
            term.value["type"] = "arr";
            get_token();
        } else if (current_token == Token("symbol", "{")) {  // type = dict
            term.children.push_back(parse_dict());
            term.value["type"] = "dict";
            get_token();
        } else if (current_token.type == "identifier" || current_token == Tokens("keyword", BUILTINTYPE)) {  // type = variable or call or type
            Node var("variable", {{"var", current_token.value}, {"state", "false"}, {"name", "var"}});
            get_token();
            term.value["type"] = "variable";
            if (current_token == Tokens("symbol", {"[", "."})) {
                Node t = parse_variable(var);
                term.value["type"] = t.type;
                term.children.push_back(t);
                get_token();
            } else
                term.children.push_back(var);
            if (current_token == Tokens("symbol", {"<", "("})) {
                Node t = parse_call(term.children.back());
                get_token();
                term.value["type"] = "call";
                term.children = t.children;
            }
        } else if (current_token.type == "int") {  // type = int
            term.value["type"] = "int";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "float") {  // type = float
            term.value["type"] = "float";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token == Tokens("keyword", {"true", "false"})) {  // type = bool
            term.value["type"] = "bool";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "char") {  // type = char
            term.value["type"] = "char";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "str") {  // type = str
            term.value["type"] = "str";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token == Token("keyword", "void")) {  // type = void
            term.value["type"] = "void";
            term.value["value"] = current_token.value;
            get_token();
        } else if (current_token == Token("keyword", "null")) {  // type = null
            term.value["type"] = "null";
            term.value["value"] = current_token.value;
            get_token();
        } else {
            parser_error("Expected term, not " + current_token.toString() + " (1)");
        }
    } else
        parser_error("Expected term, not " + current_token.toString() + " (0)");
    add_call_stack("parse_term", 1);
    return term;
}

Node Parser::parse_variable(const Node& var) {
    add_call_stack("parse_variable", 0);
    Node variable("variable", {}, var);
    if (current_token == Token("symbol", "[")) {
        variable.value["state"] = "index";
        get_token();
        variable.children.push_back(parse_expression());
        if (current_token != Token("symbol", "]"))
            parser_error("Expected ']', not " + current_token.toString());
        variable.value["name"] = "index";
    } else if (current_token == Token("symbol", ".")) {
        variable.value["state"] = "attr";
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        variable.value["name"] = current_token.value;
    } else
        parser_error("Expected '[' or '.', not " + current_token.toString());
    if (next_token() == Tokens("symbol", {".", "["})) {
        get_token();
        variable = parse_variable(variable);
    }
    if (next_token() == Tokens("symbol", {"<", "("})) {
        get_token();
        variable = parse_call(variable);
    }
    add_call_stack("parse_variable", 1);
    return variable;
}

Node Parser::parse_use_typevar() {
    add_call_stack("parse_use_typevar", 0);
    Node typevar("use_typevar");
    if (current_token == Token("symbol", "<")) {
        do {
            get_token();
            typevar.children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
        get_token();
    }
    add_call_stack("parse_use_typevar", 1);
    return typevar;
}

Node Parser::parse_declare_typevar() {
    add_call_stack("parse_declare_typevar", 0);
    Node typevar("typevar");
    if (current_token == Token("symbol", "<")) {
        do {
            get_token();
            if (current_token.type != "identifier")
                parser_error("Expected identifier, not " + current_token.toString());
            typevar.children.push_back(Node("declare_typevar", {{"name", current_token.value}}));
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
        get_token();
    }
    add_call_stack("parse_declare_typevar", 1);
    return typevar;
}

Node Parser::parse_call(const Node& var) {
    add_call_stack("parse_call", 0);
    Node call("call", {}, var);
    call.children.push_back(parse_use_typevar());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    if (next_token() == Token("symbol", ")")) {
        get_token();
        return call;
    }
    Node args_call("args_call");
    do {
        get_token();
        args_call.children.push_back(parse_expression());
    } while (current_token == Token("symbol", ","));
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    call.children.push_back(args_call);
    if (next_token() == Tokens("symbol", {".", "["})) {
        get_token();
        call = parse_variable(call);
    }
    if (next_token() == Tokens("symbol", {"<", "("})) {
        get_token();
        call = parse_call(call);
    }
    add_call_stack("parse_call", 1);
    return call;
}

Node Parser::parse_function() {
    add_call_stack("parse_function", 0);
    get_token();
    Node func("func");
    if (current_token == Token("keyword", "const")) {
        func.value["const"] = "true";
        get_token();
    } else
        func.value["const"] = "false";
    func.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    func.value["name"] = current_token.value;
    get_token();
    func.children.push_back(parse_declare_typevar());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    Node args_declare("args_declare");
    for (const Node& i : parse_declare_args())
        args_declare.children.push_back(i);
    func.children.push_back(args_declare);
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    func.children.push_back(parse_statements());
    add_call_stack("parse_function", 1);
    return func;
}

Node Parser::parse_class() {
    add_call_stack("parse_class", 0);
    get_token();
    Node class_node("class");
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    class_node.value["name"] = current_token.value;
    get_token();
    class_node.children.push_back(parse_declare_typevar());
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    while (current_token != Token("symbol", "}")) {
        if (current_token == Tokens("keyword", {"attr", "static"})) {
            for (const Node& i : parse_statement())
                class_node.children.push_back(i);
        } else if (current_token == Token("keyword", "func"))
            class_node.children.push_back(parse_function());
        else if (current_token == Token("keyword", "method"))
            class_node.children.push_back(parse_method());
        get_token();
    }
    add_call_stack("parse_class", 1);
    return class_node;
}

Node Parser::parse_method() {
    add_call_stack("parse_method", 0);
    get_token();
    Node method("method");
    if (current_token == Token("keyword", "static")) {
        method.value["static"] = "true";
        get_token();
    } else
        method.value["static"] = "false";
    if (current_token == Token("keyword", "public")) {
        method.value["modifier"] = "public";
        get_token();
    } else
        method.value["modifier"] = "local";
    method.children.push_back(parse_type());
    get_token();
    if (current_token.type == "identifier") {
        method.value["name"] = current_token.value;
        method.value["operator"] = "false";
    } else if (current_token.type == "str") {
        method.value["name"] = current_token.value;
        method.value["operator"] = "true";
    } else
        parser_error("Expected identifier or string, not " + current_token.toString());
    get_token();
    method.children.push_back(parse_declare_typevar());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    if (next_token() == Token("identifier", "self")) {
        get_token();
        method.value["self"] = "true";
        get_token();
    }
    Node args("args");
    if (current_token == Token("symbol", ",")) {
        for (const Node& i : parse_declare_args())
            args.children.push_back(i);
    } else if (current_token != Token("symbol", ")"))
        parser_error("Expected ',' or ')', not " + current_token.toString());
    method.children.push_back(args);
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    method.children.push_back(parse_statements());
    add_call_stack("parse_method", 1);
    return method;
}

vector<Node> Parser::parse_declare_args() {
    add_call_stack("parse_declare_args", 0);
    vector<Node> args;
    Node type;
    if (next_token() == Token("symbol", ")")) {
        get_token();
        add_call_stack("parse_declare_args", 1);
        return {};
    }
    string tuple;
    do {
        tuple = "false";
        get_token();
        if (current_token == Token("symbol", "*")) {
            get_token();
            tuple = "true";
        }
        type = parse_type();
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        args.push_back(Node("declare_arg", {{"name", current_token.value}, {"tuple", tuple}}, {type}));
        get_token();
    } while (current_token == Token("symbol", ","));
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    add_call_stack("parse_declare_args", 1);
    return args;
}

Node Parser::parse_arr() {
    add_call_stack("parse_arr", 0);
    Node arr("arr");
    get_token();
    while (current_token != Token("symbol", "]")) {
        arr.children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token != Token("symbol", "]"))
            parser_error("Expected ',' or ']', not " + current_token.toString());
    }
    add_call_stack("parse_arr", 1);
    return arr;
}

Node Parser::parse_tuple() {
    add_call_stack("parse_tuple", 0);
    Node tuple("tuple");
    get_token();
    while (current_token != Token("symbol", ")")) {
        tuple.children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token == Token("symbol", ")"))
            break;
        else
            parser_error("Expected ',' or ')', not " + current_token.toString());
    }
    if (tuple.children.size() == 1) {
        add_call_stack("parse_tuple", 1);
        return tuple.children[0];
    }
    tuple.value["length"] = to_string(tuple.children.size());
    add_call_stack("parse_tuple", 1);
    return tuple;
}

Node Parser::parse_dict() {
    add_call_stack("parse_dict", 0);
    Node dict("dict");
    get_token();
    while (current_token != Token("symbol", "}")) {
        dict.children.push_back(parse_expression());
        if (current_token == Token("symbol", ":")) {
            get_token();
            dict.children.push_back(parse_expression());
        } else
            parser_error("Expected ':', not " + current_token.toString());
        if (current_token == Token("symbol", "}"))
            break;
        else if (current_token != Token("symbol", ","))
            parser_error("Expected ',' or '}', not " + current_token.toString());
    }
    add_call_stack("parse_dict", 1);
    return dict;
}

Node Parser::parse_statements() {
    add_call_stack("parse_statements", 0);
    Node statements("statements");
    while (current_token != Token("symbol", "}")) {
        for (const Node& i : parse_statement())
            statements.children.push_back(i);
        get_token();
    }
    add_call_stack("parse_statements", 1);
    return statements;
}

vector<Node> Parser::parse_statement() {
    if (current_token == Token("keyword", "if"))
        return {parse_if()};
    else if (current_token == Token("keyword", "for"))
        return {parse_for()};
    else if (current_token == Token("keyword", "while"))
        return {parse_while()};
    else if (current_token == Token("keyword", "break"))
        return {parse_break()};
    else if (current_token == Token("keyword", "return"))
        return {parse_return()};
    else if (current_token == Token("keyword", "continue"))
        return {Node("continue")};
    else if (current_token == Tokens("keyword", {"var", "const"}))
        return parse_declare(false);
    else if (current_token == Tokens("keyword", {"attr", "static"}))
        return parse_declare(true);
    else if (is_term(current_token)) {
        Node t = parse_expression();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
        return {t};
    } else if (current_token != Token("keyword", "pass"))
        parser_error("Expected statement, not " + current_token.toString());
    add_call_stack("parse_statement", 2);
    return {};
}

Node Parser::parse_if() {
    add_call_stack("parse_if", 0);
    get_token();
    Node if_node("if");
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    if_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    if_node.children.push_back(parse_statements());
    int n = 0;
    while (next_token() == Token("keyword", "elif")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "("))
            parser_error("Expected '(', not " + current_token.toString());
        get_token();
        if_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ")"))
            parser_error("Expected ')', not " + current_token.toString());
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        if_node.children.push_back(parse_statements());
        n++;
    }
    if_node.value["elif_n"] = to_string(n);
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        if_node.children.push_back(parse_statements());
        if_node.value["else"] = "true";
    } else
        if_node.value["else"] = "false";
    add_call_stack("parse_if", 1);
    return if_node;
}

Node Parser::parse_for() {
    add_call_stack("parse_for", 0);
    get_token();
    Node for_node("for");
    if (current_token.type == "identifier") {
        for_node.value["label"] = current_token.value;
        get_token();
    } else
        for_node.value["label"] = "for";
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    for_node.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    for_node.value["name"] = current_token.value;
    get_token();
    if (current_token != Token("keyword", "in"))
        parser_error("Expected 'in', not " + current_token.toString());
    get_token();
    for_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    for_node.children.push_back(parse_statements());
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        for_node.children.push_back(parse_statements());
        for_node.value["else"] = "true";
    } else
        for_node.value["else"] = "false";
    add_call_stack("parse_for", 1);
    return for_node;
}

Node Parser::parse_while() {
    add_call_stack("parse_while", 0);
    get_token();
    Node while_node("while");
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    while_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    while_node.children.push_back(parse_statements());
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        while_node.children.push_back(parse_statements());
        while_node.value["else"] = "true";
    } else
        while_node.value["else"] = "false";
    add_call_stack("parse_while", 1);
    return while_node;
}

Node Parser::parse_break() {
    add_call_stack("parse_break", 0);
    get_token();
    Node break_node("break");
    if (current_token.type == "identifier") {
        break_node.value["label"] = current_token.value;
        get_token();
    }
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';', not " + current_token.toString());
    add_call_stack("parse_break", 1);
    return break_node;
}

Node Parser::parse_return() {
    add_call_stack("parse_return", 0);
    get_token();
    Node return_node("return");
    if (current_token == Token("symbol", ";"))
        return return_node;
    else {
        return_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
    }
    add_call_stack("parse_return", 1);
    return return_node;
}
