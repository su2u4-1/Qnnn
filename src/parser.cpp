#include "../include/parser.h"

#include "../include/qlib.h"

Parser::Parser(const vector<Token>& tokens, const string& file_name) {
    if (!tokens.empty()) {
        this->tokens = tokens;
        this->file_name = file_name;
        this->index = 0;
        this->current_token = tokens[index];
    } else {
        cout << "Error: No tokens provided." << endl;
        exit(1);
    }
}

Token Parser::next_token() {
    if (index < tokens.size()) {
        return tokens[index];
    } else {
        return Token("EOF", "", file_name, {-1, -1});
    }
}

void Parser::get_token() {
    if (index < tokens.size()) {
        current_token = tokens[index];
        index++;
        while (current_token.type == "comment") {
            current_token = tokens[index];
            index++;
        }
    } else {
        current_token = Token("EOF", "", file_name, {-1, -1});
    }
}

Node Parser::parse() {
    Node root("program");
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import")) {
            root.children.push_back(parse_import());
        } else {
            for (const Node& i : parse_statement()) {
                root.children.push_back(i);
            }
        }
    }
    return root;
}

Node Parser::parse_import() {
    get_token();
    Node import("import");
    if (current_token == Tokens("identifier", STDLIB)) {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";")) {
            error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        import.value["alias"] = "stdlib";
        return import;
    } else if (current_token.type == "string") {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("keyword", "as")) {
            error("Expected keyword 'as'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        import.value["alias"] = current_token.value;
        return import;
    } else {
        error("Expected stdlib or string", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return Node();
}

vector<Node> Parser::parse_declare_var() {
    map<string, string> state;
    vector<Node> nodes;
    if (current_token == Token("keyword", "var")) {
        state["kind"] = "var";
    } else if (current_token == Token("keyword", "constant")) {
        state["kind"] = "constant";
    } else {
        error("Expected 'var' or 'constant'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if (current_token == Token("keyword", "global")) {
        state["modifier"] = "global";
        get_token();
    } else {
        state["modifier"] = "local";
    }
    Node type = parse_type();
    get_token();
    if (current_token.type != "identifier") {
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    state["name"] = current_token.value;
    get_token();
    Node expression("expression");
    if (current_token == Token("symbol", "=")) {
        get_token();
        expression = parse_expression();
    }
    nodes.push_back(Node("declare_var", state, {type, expression}));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        state["name"] = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        nodes.push_back(Node("declare_var", state, {type, expression}));
    }
    if (current_token != Token("symbol", ";")) {
        error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return nodes;
}

vector<Node> Parser::parse_declare_attr(const string& class_name) {
    map<string, string> state = {{"class", class_name}};
    vector<Node> nodes;
    if (current_token == Token("keyword", "attr")) {
        state["kind"] = "attr";
    } else if (current_token == Token("keyword", "static")) {
        state["kind"] = "static";
    } else {
        error("Expected 'attr' or 'static'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if (current_token == Token("keyword", "public")) {
        state["modifier"] = "public";
        get_token();
    } else {
        state["modifier"] = "local";
    }
    Node type = parse_type();
    get_token();
    if (current_token.type != "identifier") {
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    state["name"] = current_token.value;
    get_token();
    Node expression("expression");
    if (current_token == Token("symbol", "=")) {
        get_token();
        expression = parse_expression();
    }
    nodes.push_back(Node("declare_attr", state, {type, expression}));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        state["name"] = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        nodes.push_back(Node("declare_attr", state, {type, expression}));
    }
    if (current_token != Token("symbol", ";")) {
        error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return nodes;
}

Node Parser::parse_type() {
    Node type("type");
    if (current_token == Tokens("keyword", BUILTINTYPE) || current_token.type == "identifier") {
        type.value["name"] = current_token.value;
    } else {
        error("Expected built-in type or identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    if (next_token() == Token("symbol", "<")) {
        get_token();
        do {
            get_token();
            type.children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">")) {
            error("Expected '>'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    return type;
}

Node Parser::parse_expression() {
}

Node Parser::parse_term() {
}

Node Parser::parse_variable() {
}

Node Parser::parse_call() {
}

Node Parser::parse_function() {
    get_token();
    Node func("function");
    if (current_token == Token("keyword", "constant")) {
        func.value["constant"] = "true";
    } else {
        func.value["constant"] = "false";
    }
    get_token();
    func.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier") {
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    func.value["name"] = current_token.value;
    get_token();
    int n = 0;
    if (current_token == Token("symbol", "<")) {
        do {
            get_token();
            func.children.push_back(parse_type());
            n++;
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">")) {
            error("Expected '>'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    func.value["type_n"] = to_string(n);
    if (current_token != Token("symbol", "(")) {
        error("Expected '('", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    Node args("args");
    for (const Node& i : parse_args()) {
        args.children.push_back(i);
    }
    func.children.push_back(args);
    get_token();
    if (current_token != Token("symbol", "{")) {
        error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    Node statements("statements");
    do {
        for (const Node& i : parse_statement())
            statements.children.push_back(i);
        get_token();
    } while (current_token != Token("symbol", "}"));
}

Node Parser::parse_class() {
}

Node Parser::parse_method() {
}

vector<Node> Parser::parse_args() {
    vector<Node> args;
    Node type;
    do {
        get_token();
        type = parse_type();
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        args.push_back(Node("declare_arg", {{"name", current_token.value}}, {type}));
        get_token();
    } while (current_token == Token("symbol", ","));
    if (current_token != Token("symbol", ")")) {
        error("Expected ')'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return args;
}

Node Parser::parse_arr() {
    get_token();
    Node arr("arr");
    get_token();
    while (current_token != Token("symbol", "]")) {
        arr.children.push_back(parse_expression());
        if (current_token == Token("symbol", ",")) {
            get_token();
        } else if (current_token != Token("symbol", "]")) {
            error("Expected ',' or ']'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    return arr;
}

Node Parser::parse_tuple() {
    get_token();
    Node tuple("tuple");
    get_token();
    while (current_token != Token("symbol", "]")) {
        tuple.children.push_back(parse_expression());
        if (current_token == Token("symbol", ",")) {
            get_token();
        } else if (current_token != Token("symbol", "]")) {
            error("Expected ',' or ']'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    tuple.value["length"] = to_string(tuple.children.size());
    return tuple;
}

Node Parser::parse_dict() {
    get_token();
    Node dict("dict");
    get_token();
    while (current_token != Token("symbol", "}")) {
        dict.children.push_back(parse_expression());
        if (current_token == Token("symbol", ":")) {
            get_token();
            dict.children.push_back(parse_expression());
        } else {
            error("Expected ':'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        if (current_token == Token("symbol", "}")) {
            break;
        } else if (current_token != Token("symbol", ",")) {
            error("Expected ',' or '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    return dict;
}

vector<Node> Parser::parse_statement() {
    if (current_token == Token("keyword", "if")) {
        return {parse_if()};
    } else if (current_token == Token("keyword", "for")) {
        return {parse_for()};
    } else if (current_token == Token("keyword", "while")) {
        return {parse_while()};
    } else if (current_token == Token("keyword", "break")) {
        return {parse_break()};
    } else if (current_token == Token("keyword", "return")) {
        return {parse_return()};
    } else if (current_token == Token("keyword", "continue")) {
        return {Node("continue")};
    } else if (current_token == Tokens("keyword", {"var", "constant"})) {
        return parse_declare_var();
    } else if (is_term(current_token)) {
        Node t = parse_expression();
        if (current_token != Token("symbol", ";")) {
            error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        return {t};
    } else if (current_token == Token("keyword", "pass")) {
        error("Expected statement", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return {};
}

Node Parser::parse_if() {
    get_token();
    Node if_node("if");
    if (current_token != Token("symbol", "(")) {
        error("Expected '('", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")")) {
        error("Expected ')'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    if (current_token != Token("symbol", "{")) {
        error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}")) {
        for (const Node& i : parse_statement()) {
            statements.children.push_back(i);
        }
    }
    if_node.children.push_back(statements);
    if (current_token != Token("symbol", "}")) {
        error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    int n = 0;
    while (next_token() == Token("keyword", "elif")) {
        get_token();
        if (current_token != Token("symbol", "(")) {
            error("Expected '('", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        if_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ")")) {
            error("Expected ')'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        if (current_token != Token("symbol", "{")) {
            error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        statements = Node("statements");
        while (current_token != Token("symbol", "}")) {
            for (const Node& i : parse_statement()) {
                statements.children.push_back(i);
            }
        }
        if_node.children.push_back(statements);
        if (current_token != Token("symbol", "}")) {
            error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        n++;
    }
    if_node.value["elif_n"] = to_string(n);
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{")) {
            error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        statements = Node("statements");
        while (current_token != Token("symbol", "}")) {
            for (const Node& i : parse_statement()) {
                statements.children.push_back(i);
            }
        }
        if_node.children.push_back(statements);
        if (current_token != Token("symbol", "}")) {
            error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        if_node.value["else"] = "true";
    }
    return if_node;
}

Node Parser::parse_for() {
    get_token();
    Node for_node("for");
    if (current_token.type == "identifier") {
        for_node.value["label"] = current_token.value;
        get_token();
    } else {
        for_node.value["label"] = "for";
    }
    if (current_token != Token("symbol", "(")) {
        error("Expected '('", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    for_node.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier") {
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    for_node.value["name"] = current_token.value;
    get_token();
    if (current_token != Token("symbol", "in")) {
        error("Expected 'in'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    for_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")")) {
        error("Expected ')'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if (current_token != Token("symbol", "{")) {
        error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}")) {
        for (const Node& i : parse_statement()) {
            statements.children.push_back(i);
        }
    }
    for_node.children.push_back(statements);
    if (current_token != Token("symbol", "}")) {
        error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{")) {
            error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        Node statements("statements");
        while (current_token != Token("symbol", "}")) {
            for (const Node& i : parse_statement()) {
                statements.children.push_back(i);
            }
        }
        for_node.children.push_back(statements);
        if (current_token != Token("symbol", "}")) {
            error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        for_node.value["else"] = "true";
    }
    return for_node;
}

Node Parser::parse_while() {
    get_token();
    Node while_node("while");
    if (current_token != Token("symbol", "(")) {
        error("Expected '('", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    while_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")")) {
        error("Expected ')'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if (current_token != Token("symbol", "{")) {
        error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}")) {
        for (const Node& i : parse_statement()) {
            statements.children.push_back(i);
        }
    }
    while_node.children.push_back(statements);
    if (current_token != Token("symbol", "}")) {
        error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{")) {
            error("Expected '{'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        get_token();
        Node statements("statements");
        while (current_token != Token("symbol", "}")) {
            for (const Node& i : parse_statement()) {
                statements.children.push_back(i);
            }
        }
        while_node.children.push_back(statements);
        if (current_token != Token("symbol", "}")) {
            error("Expected '}'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        while_node.value["else"] = "true";
    }
    return while_node;
}

Node Parser::parse_break() {
    get_token();
    Node break_node("break");
    if (current_token.type == "identifier") {
        break_node.value["label"] = current_token.value;
        get_token();
    }
    if (current_token != Token("symbol", ";")) {
        error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
}

Node Parser::parse_return() {
    get_token();
    Node return_node("return");
    if (current_token == Token("symbol", ";")) {
        return return_node;
    } else {
        return_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ";")) {
            error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    return return_node;
}
