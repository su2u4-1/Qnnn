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
    } else {
        current_token = Token("EOF", "", file_name, {-1, -1});
    }
}

Node Parser::parse() {
    Node root("program", {}, vector<Node>());
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import")) {
            root.children.push_back(parse_import());
        } else if (current_token == Tokens("keyword", {"var", "constant"})) {
            for (Node i : parse_declare()) {
                root.children.push_back(i);
            }
        } else if (current_token == Token("keyword", "class")) {
            root.children.push_back(parse_class());
        } else if (current_token == Token("keyword", "function")) {
            root.children.push_back(parse_function());
        } else if (current_token == Tokens("keyword", {"if", "while", "for"})) {
            root.children.push_back(parse_statement());
        } else if (current_token.type == "identifier") {
            root.children.push_back(parse_expression());
        } else {
            error("Unexpected token", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    }
    return root;
}

Node Parser::parse_import() {
    get_token();
    Node import("import", {});
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
}

vector<Node> Parser::parse_declare() {
    map<string, string> state;
    vector<Node> nodes;
    if (current_token == Token("keyword", "var")) {
        state["kind"] = "var";
    } else if (current_token == Token("keyword", "constant")) {
        state["kind"] = "constant";
    } else if (current_token == Token("keyword", "attr")) {
        state["kind"] = "attr";
    } else if (current_token == Token("keyword", "static")) {
        state["kind"] = "static";
    } else {
        error("Expected 'var', 'constant', 'attr', or 'static'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    get_token();
    if (current_token == Token("keyword", "global")) {
        if (state["kind"] == "var" || state["kind"] == "constant") {
            state["modifier"] = "global";
            get_token();
        } else {
            error("'attr' or 'static' should not be followed by 'global'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    } else if (current_token == Token("keyword", "public")) {
        if (state["kind"] == "attr" || state["kind"] == "static") {
            state["modifier"] = "public";
            get_token();
        } else {
            error("'var' or 'constant' should not be followed by 'public'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
    } else {
        if (state["kind"] == "var" || state["kind"] == "constant") {
            state["modifier"] = "local";
        } else {
            state["modifier"] = "private";
        }
    }
    Node type = parse_type();
    get_token();
    if (current_token.type != "identifier") {
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    string name = current_token.value;
    get_token();
    Node expression;
    if (current_token == Token("symbol", "=")) {
        get_token();
        expression = parse_expression();
    }
    nodes.push_back(Node("declare", state, {Node("identifier", {{"name", name}}), type, expression}));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
        }
        name = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        nodes.push_back(Node("declare", state, {Node("identifier", {{"name", name}}), type, expression}));
    }
    if (current_token != Token("symbol", ";")) {
        error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_getitem(file_name, current_token.line - 1));
    }
    return nodes;
}

Node Parser::parse_type() {
    Node type("type", {});
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
}

Node Parser::parse_class() {
}

Node Parser::parse_method() {
}

Node Parser::parse_args() {
}

Node Parser::parse_arr() {
}

Node Parser::parse_tuple() {
}

Node Parser::parse_dict() {
}

Node Parser::parse_statement() {
}

Node Parser::parse_if() {
}

Node Parser::parse_for() {
}

Node Parser::parse_while() {
}

Node Parser::parse_break() {
}

Node Parser::parse_return() {
}
