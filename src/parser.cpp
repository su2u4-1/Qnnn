#include "../include/parser.h"

#include "../include/qlib.h"

Parser::Parser(vector<Token> tokens, string file_name) {
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
    Node root("Program", {}, vector<Node>());
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import")) {
            root.children.push_back(parse_import());
        } else if (current_token == Tokens("keyword", {"var", "constant"})) {
            for (Node i : parse_declare()) {
                root.children.push_back(i);
            }
        }
    }
    return root;
}

Node Parser::parse_import() {
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
        error("Expected 'var', 'constant', 'attr', or 'static'", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
    }
    get_token();
    if (current_token == Token("keyword", "global")) {
        if (state["kind"] == "var" || state["kind"] == "constant") {
            state["modifier"] = "global";
            get_token();
        } else {
            error("'attr' or 'static' should not be followed by 'global'", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
        }
    } else if (current_token == Token("keyword", "public")) {
        if (state["kind"] == "attr" || state["kind"] == "static") {
            state["modifier"] = "public";
            get_token();
        } else {
            error("'var' or 'constant' should not be followed by 'public'", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
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
        error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
    }
    string name = current_token.value;
    get_token();
    Node expression;
    if (current_token == Token("symbol", "=")) {
        get_token();
        expression = parse_expression();
    }
    nodes.push_back(Node("Declare", state, {Node("Identifier", {{"name", name}}), type, expression}));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier") {
            error("Expected identifier", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
        }
        name = current_token.value;
        get_token();
        expression = Node();
        if (current_token == Token("symbol", "=")) {
            get_token();
            expression = parse_expression();
        }
        nodes.push_back(Node("Declare", state, {Node("Identifier", {{"name", name}}), type, expression}));
    }
    if (current_token != Token("symbol", ";")) {
        error("Expected ';'", file_name, {current_token.line, current_token.column}, source_code_map[file_name][current_token.line - 1]);
    }
    return nodes;
}

Node Parser::parse_type() {
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
