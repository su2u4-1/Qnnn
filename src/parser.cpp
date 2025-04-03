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
    if (index < tokens.size())
        return tokens[index];
    else
        return Token("EOF", "", file_name, {-1, -1});
}

void Parser::get_token() {
    if (index < tokens.size()) {
        current_token = tokens[index];
        index++;
        while (current_token.type == "comment") {
            current_token = tokens[index];
            index++;
        }
    } else
        current_token = Token("EOF", "", file_name, {-1, -1});
}

void Parser::parser_error(const string& msg) {
    parser_error(msg, current_token);
}

void Parser::parser_error(const string& msg, const Token& token) {
    error(msg, file_name, {token.line, token.column}, source_code_getitem(file_name, token.line - 1));
}

Node Parser::parse() {
    Node root("program");
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import"))
            root.children.push_back(parse_import());
        else
            for (const Node& i : parse_statement()) root.children.push_back(i);
    }
    return root;
}

Node Parser::parse_import() {
    get_token();
    Node import("import");
    if (current_token == Tokens("identifier", STDLIB)) {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';'");
        import.value["alias"] = "stdlib";
        return import;
    } else if (current_token.type == "string") {
        import.value["name"] = current_token.value;
        get_token();
        if (current_token != Token("keyword", "as"))
            parser_error("Expected keyword 'as'");
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier");
        import.value["alias"] = current_token.value;
        return import;
    } else
        parser_error("Expected stdlib or string");
    return Node();
}

vector<Node> Parser::parse_declare(bool attr) {
    map<string, string> state;
    vector<Node> nodes;
    if (attr) {
        if (current_token == Token("keyword", "attr"))
            state["kind"] = "attr";
        else if (current_token == Token("keyword", "static"))
            state["kind"] = "static";
        else
            parser_error("Expected 'attr' or 'static'");
        get_token();
        if (current_token == Token("keyword", "public")) {
            state["modifier"] = "public";
            get_token();
        } else
            state["modifier"] = "local";
    } else {
        if (current_token == Token("keyword", "var"))
            state["kind"] = "var";
        else if (current_token == Token("keyword", "constant"))
            state["kind"] = "constant";
        else
            parser_error("Expected 'var' or 'constant'");
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
        parser_error("Expected identifier");
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
        if (current_token.type != "identifier")
            parser_error("Expected identifier");
        state["name"] = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        nodes.push_back(Node("declare_var", state, {type, expression}));
    }
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';'");
    return nodes;
}

Node Parser::parse_type() {
    Node type("type");
    if (current_token == Tokens("keyword", BUILTINTYPE) || current_token.type == "identifier")
        type.value["name"] = current_token.value;
    else
        parser_error("Expected built-in type or identifier");
    if (next_token() == Token("symbol", "<")) {
        get_token();
        do {
            get_token();
            type.children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>'");
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
    if (current_token == Token("keyword", "constant"))
        func.value["constant"] = "true";
    else
        func.value["constant"] = "false";
    get_token();
    func.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier");
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
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>'");
    }
    func.value["type_n"] = to_string(n);
    if (current_token != Token("symbol", "("))
        parser_error("Expected '('");
    Node args("args");
    for (const Node& i : parse_args())
        args.children.push_back(i);
    func.children.push_back(args);
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{'");
    Node statements("statements");
    do {
        for (const Node& i : parse_statement())
            statements.children.push_back(i);
        get_token();
    } while (current_token != Token("symbol", "}"));
}

Node Parser::parse_class() {
    get_token();
    Node class_node("class");
    if (current_token.type != "identifier")
        parser_error("Expected identifier");
    class_node.value["name"] = current_token.value;
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{'");
    while (current_token != Token("symbol", "}")) {
        if (current_token == Tokens("keyword", {"attr", "static"})) {
            for (const Node& i : parse_statement())
                class_node.children.push_back(i);
        } else if (current_token == Token("keyword", "function"))
            class_node.children.push_back(parse_function());
        else if (current_token == Token("keyword", "method"))
            class_node.children.push_back(parse_method());
        get_token();
    }
    return class_node;
}

Node Parser::parse_method() {
    // TODO: 'method' ['static'] ['public'] <type> (<identifier'method_name'> | <str'operator'>) ['<' <identifier'type_name'> [{',' <identifier'type_name'>}] '>'] '(' [<identifier'self'>] [',' <declare_args>] ')' '{' {<statement>} '}'
}

vector<Node> Parser::parse_args() {
    vector<Node> args;
    Node type;
    do {
        get_token();
        type = parse_type();
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier");
        args.push_back(Node("declare_arg", {{"name", current_token.value}}, {type}));
        get_token();
    } while (current_token == Token("symbol", ","));
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')'");
    return args;
}

Node Parser::parse_arr() {
    get_token();
    Node arr("arr");
    get_token();
    while (current_token != Token("symbol", "]")) {
        arr.children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token != Token("symbol", "]"))
            parser_error("Expected ',' or ']'");
    }
    return arr;
}

Node Parser::parse_tuple() {
    get_token();
    Node tuple("tuple");
    get_token();
    while (current_token != Token("symbol", "]")) {
        tuple.children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token != Token("symbol", "]"))
            parser_error("Expected ',' or ']'");
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
        } else
            parser_error("Expected ':'");
        if (current_token == Token("symbol", "}"))
            break;
        else if (current_token != Token("symbol", ","))
            parser_error("Expected ',' or '}'");
    }
    return dict;
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
    else if (current_token == Tokens("keyword", {"var", "constant"}))
        return parse_declare(false);
    else if (current_token == Tokens("keyword", {"attr", "static"}))
        return parse_declare(true);
    else if (is_term(current_token)) {
        Node t = parse_expression();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';'");
        return {t};
    } else if (current_token == Token("keyword", "pass"))
        parser_error("Expected statement");
    return {};
}

Node Parser::parse_if() {
    get_token();
    Node if_node("if");
    if (current_token != Token("symbol", "("))
        parser_error("Expected '('");
    get_token();
    if_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')'");
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{'");
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}"))
        for (const Node& i : parse_statement()) statements.children.push_back(i);
    if_node.children.push_back(statements);
    if (current_token != Token("symbol", "}"))
        parser_error("Expected '}'");
    int n = 0;
    while (next_token() == Token("keyword", "elif")) {
        get_token();
        if (current_token != Token("symbol", "("))
            parser_error("Expected '('");
        get_token();
        if_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ")"))
            parser_error("Expected ')'");
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{'");
        get_token();
        statements = Node("statements");
        while (current_token != Token("symbol", "}"))
            for (const Node& i : parse_statement()) statements.children.push_back(i);
        if_node.children.push_back(statements);
        if (current_token != Token("symbol", "}"))
            parser_error("Expected '}'");
        n++;
    }
    if_node.value["elif_n"] = to_string(n);
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{'");
        get_token();
        statements = Node("statements");
        while (current_token != Token("symbol", "}"))
            for (const Node& i : parse_statement()) statements.children.push_back(i);
        if_node.children.push_back(statements);
        if (current_token != Token("symbol", "}"))
            parser_error("Expected '}'");
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
    } else
        for_node.value["label"] = "for";
    if (current_token != Token("symbol", "("))
        parser_error("Expected '('");
    get_token();
    for_node.children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier");
    for_node.value["name"] = current_token.value;
    get_token();
    if (current_token != Token("symbol", "in"))
        parser_error("Expected 'in'");
    get_token();
    for_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')'");
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{'");
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}"))
        for (const Node& i : parse_statement()) statements.children.push_back(i);
    for_node.children.push_back(statements);
    if (current_token != Token("symbol", "}"))
        parser_error("Expected '}'");
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{'");
        get_token();
        Node statements("statements");
        while (current_token != Token("symbol", "}"))
            for (const Node& i : parse_statement()) statements.children.push_back(i);
        for_node.children.push_back(statements);
        if (current_token != Token("symbol", "}"))
            parser_error("Expected '}'");
        for_node.value["else"] = "true";
    }
    return for_node;
}

Node Parser::parse_while() {
    get_token();
    Node while_node("while");
    if (current_token != Token("symbol", "("))
        parser_error("Expected '('");
    get_token();
    while_node.children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')'");
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{'");
    get_token();
    Node statements("statements");
    while (current_token != Token("symbol", "}"))
        for (const Node& i : parse_statement()) statements.children.push_back(i);
    while_node.children.push_back(statements);
    if (current_token != Token("symbol", "}"))
        parser_error("Expected '}'");
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{'");
        get_token();
        Node statements("statements");
        while (current_token != Token("symbol", "}"))
            for (const Node& i : parse_statement()) statements.children.push_back(i);
        while_node.children.push_back(statements);
        if (current_token != Token("symbol", "}"))
            parser_error("Expected '}'");
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
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';'");
    return break_node;
}

Node Parser::parse_return() {
    get_token();
    Node return_node("return");
    if (current_token == Token("symbol", ";"))
        return return_node;
    else {
        return_node.children.push_back(parse_expression());
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';'");
    }
    return return_node;
}
