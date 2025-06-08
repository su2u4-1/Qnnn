#include "../include/parser.h"

#include "../include/qlib.h"

Parser::Parser(const vector<shared_ptr<Token>>& tokens, const fs::path& file_name, Log& log) {
    if (!tokens.empty()) {
        this->tokens = tokens;
        this->file_name = file_name;
        this->index = 0;
        this->current_token = Token();
        this->log = log;
        get_token();
    } else {
        throw runtime_error("Error: No tokens provided.");
    }
}

Token Parser::next_token() {
    if (index < tokens.size()) {
        Token t = *tokens[index];
        int n = 0;
        while (t.type == "comment") {
            n++;
            if (index + n >= tokens.size())
                return Token("EOF", "", file_name, {-1, -1});
            t = *tokens[index + n];
        }
        return t;
    } else
        return Token("EOF", "", file_name, {-1, -1});
}

void Parser::get_token() {
    if (index < tokens.size()) {
        current_token = *tokens[index];
        index++;
        if (current_token.type == "comment") {
            get_token();
        }
    } else
        current_token = Token("EOF", "", file_name, {-1, -1});
    log.log_msg(current_token.toString() + " " + to_string(index), 2);
}

void Parser::rollback_token() {
    if (index > 0) {
        index--;
        current_token = *tokens[index - 1];
        if (current_token.type == "comment") {
            rollback_token();
        }
    } else
        current_token = Token("SOF", "", file_name, {-1, -1});
    log.log_msg(current_token.toString() + " " + to_string(index), 5);
}

void Parser::parser_error(const string& msg) {
    parser_error(msg, current_token);
}

void Parser::parser_error(const string& msg, const Token& token) {
    error("ParseError: " + msg, file_name, token.pos, source_code_getitem(file_name, token.pos.first - 1));
}

bool Parser::isCall() {
    log.log_msg("isCall", 0);
    int i = 0, j = 0;
    bool result = false;
    while (true) {
        if (current_token != Tokens("symbol", {"<", ",", ">"}) && current_token.type != "identifier" && current_token != Tokens("keyword", BUILTINTYPE))
            break;
        else if (current_token == Token("symbol", "<"))
            i++;
        else if (current_token == Token("symbol", ">")) {
            i--;
        }
        if (i < 0)
            break;
        else if (i == 0) {
            result = true;
            break;
        }
        get_token();
        j++;
    }
    for (int k = 0; k < j; k++)
        rollback_token();
    log.log_msg("isCall", 1);
    return result;
}

shared_ptr<Node> Parser::parse() {
    log.log_msg("parse", 0);
    shared_ptr<Node> root = make_shared<Node>("program", map<string, string>{{"name", file_name}}, current_token.pos);
    while (current_token.type != "EOF") {
        if (current_token == Token("keyword", "import"))
            root->children.push_back(parse_import());
        else if (current_token == Token("keyword", "class"))
            root->children.push_back(parse_class());
        else if (current_token == Token("keyword", "func"))
            root->children.push_back(parse_function());
        else
            for (shared_ptr<Node> i : parse_declare(false, true)) root->children.push_back(i);
        get_token();
    }
    log.log_msg("parse", 1);
    return root;
}

shared_ptr<Node> Parser::parse_import() {
    log.log_msg("parse_import", 0);
    get_token();
    shared_ptr<Node> import = make_shared<Node>("import", current_token.pos);
    if (current_token == Tokens("identifier", STDLIB)) {
        import->value["name"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
        import->value["alias"] = "stdlib";
        log.log_msg("parse_import", 1);
        return import;
    } else if (current_token.type == "str") {
        import->value["name"] = current_token.value;
        get_token();
        if (current_token != Token("keyword", "as"))
            parser_error("Expected keyword 'as', not " + current_token.toString());
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        import->value["alias"] = current_token.value;
        get_token();
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
        log.log_msg("parse_import", 1);
        return import;
    } else
        parser_error("Expected stdlib or string, not " + current_token.toString());
    log.log_msg("parse_import", 3);
    return shared_ptr<Node>();
}

vector<shared_ptr<Node>> Parser::parse_declare(bool attr, bool global) {
    log.log_msg("parse_declare", 0);
    map<string, string> state;
    vector<shared_ptr<Node>> nodes;
    pair<int, int> pos = current_token.pos;
    if (current_token == Tokens("keyword", {"var", "const", "attr", "static"})) {
        if (attr && current_token == Tokens("keyword", {"var", "const"}))
            parser_error("Expected 'attr' or 'static', not " + current_token.toString());
        if (global && current_token == Tokens("keyword", {"attr", "static"}))
            parser_error("Expected 'var' or 'const', not " + current_token.toString());
        state["kind"] = current_token.value;
    } else
        parser_error("Expected 'var', 'const', 'attr' or 'static', not " + current_token.toString());
    get_token();
    if (current_token == Token("keyword", "public")) {
        if (attr)
            state["modifier"] = current_token.value;
        else
            parser_error("Expected type declare, not " + current_token.toString());
        get_token();
    } else if (global)
        state["modifier"] = "global";
    else {
        if (attr)
            state["modifier"] = "private";
        else
            state["modifier"] = "local";
    }
    shared_ptr<Node> type = parse_type();
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    state["name"] = current_token.value;
    get_token();
    vector<shared_ptr<Node>> children{type};
    if (current_token == Token("symbol", "=")) {
        get_token();
        children.push_back(parse_expression());
    }
    if (attr)
        nodes.push_back(make_shared<Node>("declare_attr", state, children, pos));
    else
        nodes.push_back(make_shared<Node>("declare_var", state, children, pos));
    while (current_token == Token("symbol", ",")) {
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        state["name"] = current_token.value;
        get_token();
        if (children.size() == 2)
            children.pop_back();
        if (current_token == Token("symbol", "=")) {
            get_token();
            children.push_back(parse_expression());
        }
        if (attr)
            nodes.push_back(make_shared<Node>("declare_attr", state, children, pos));
        else
            nodes.push_back(make_shared<Node>("declare_var", state, children, pos));
    }
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';', not " + current_token.toString());
    log.log_msg("parse_declare", 1);
    return nodes;
}

shared_ptr<Node> Parser::parse_type() {
    log.log_msg("parse_type", 0);
    shared_ptr<Node> type = make_shared<Node>("type", current_token.pos);
    if (current_token == Tokens("keyword", BUILTINTYPE) || current_token.type == "identifier")
        type->value["name"] = current_token.value;
    else
        parser_error("Expected built-in type or identifier, not " + current_token.toString());
    if (next_token() == Token("symbol", "<")) {
        get_token();
        do {
            get_token();
            type->children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
    }
    log.log_msg("parse_type", 1);
    return type;
}

shared_ptr<Node> Parser::parse_expression() {
    log.log_msg("parse_expression", 0);
    shared_ptr<Node> expression = make_shared<Node>("expression", current_token.pos);
    vector<shared_ptr<Node>> terms_operators;
    terms_operators.push_back(parse_term());
    while (true) {
        if (current_token != Tokens("symbol", OPERATOR))
            break;
        terms_operators.push_back(make_shared<Node>("operator", map<string, string>{{"value", current_token.value}}, current_token.pos));
        get_token();
        terms_operators.push_back(parse_term());
    }
    vector<shared_ptr<Node>> stack;
    for (shared_ptr<Node> i : terms_operators) {
        if (i->type == "term")
            expression->children.push_back(i);
        else if (i->type == "operator") {
            while (!stack.empty() && operator_precedence(stack.back()->value.at("value")) >= operator_precedence(i->value.at("value"))) {
                expression->children.push_back(stack.back());
                stack.pop_back();
            }
            stack.push_back(i);
        }
    }
    while (!stack.empty()) {
        expression->children.push_back(stack.back());
        stack.pop_back();
    }
    log.log_msg("parse_expression", 1);
    return expression;
}

shared_ptr<Node> Parser::parse_term() {
    log.log_msg("parse_term", 0);
    shared_ptr<Node> term = make_shared<Node>("term", current_token.pos);
    if (is_term(current_token)) {
        if (current_token == Tokens("symbol", {"^", "@", "-", "!"})) {  // type = dereference or pointer or neg or not
            if (current_token.value == "^")
                term->value["type"] = "dereference";
            else if (current_token.value == "@")
                term->value["type"] = "pointer";
            else if (current_token.value == "-")
                term->value["type"] = "neg";
            else if (current_token.value == "!")
                term->value["type"] = "not";
            get_token();
            term->children.push_back(parse_term());
        } else if (current_token == Token("symbol", "(")) {  // type = value or expression or tuple
            term->children.push_back(parse_tuple());
            term->value["type"] = term->children.back()->type;
            get_token();
        } else if (current_token == Token("symbol", "[")) {  // type = arr
            term->children.push_back(parse_arr());
            term->value["type"] = "arr";
            get_token();
        } else if (current_token == Token("symbol", "{")) {  // type = dict
            term->children.push_back(parse_dict());
            term->value["type"] = "dict";
            get_token();
        } else if (current_token.type == "identifier" || current_token == Tokens("keyword", BUILTINTYPE)) {  // type = variable or call or type
            shared_ptr<Node> var = make_shared<Node>("variable", map<string, string>{{"state", "false"}}, make_shared<Node>("name", map<string, string>{{"name", current_token.value}}, current_token.pos), current_token.pos);
            get_token();
            term->value["type"] = "variable";
            shared_ptr<Node> t;
            if (current_token == Tokens("symbol", {"[", "."})) {
                t = parse_variable(var);
                term->value["type"] = t->type;
                term->children.push_back(t);
                get_token();
            } else if (current_token == Token("symbol", "<") && !isCall()) {
                term->children.push_back(var);
                log.log_msg("parse_term", 1);
                return term;
            } else if (current_token == Tokens("symbol", {"<", "("})) {
                t = parse_call(var);
                term->value["type"] = t->type;
                term->children.push_back(t);
                get_token();
            } else
                term->children.push_back(var);
        } else if (current_token.type == "int") {  // type = int
            term->value["type"] = "int";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "float") {  // type = float
            term->value["type"] = "float";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token == Tokens("keyword", {"true", "false"})) {  // type = bool
            term->value["type"] = "bool";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "char") {  // type = char
            term->value["type"] = "char";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token.type == "str") {  // type = str
            term->value["type"] = "str";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token == Token("keyword", "void")) {  // type = void
            term->value["type"] = "void";
            term->value["value"] = current_token.value;
            get_token();
        } else if (current_token == Token("keyword", "null")) {  // type = null
            term->value["type"] = "null";
            term->value["value"] = current_token.value;
            get_token();
        } else {
            parser_error("Expected term, not " + current_token.toString() + " (1)");
        }
    } else
        parser_error("Expected term, not " + current_token.toString() + " (0)");
    log.log_msg("parse_term", 1);
    return term;
}

shared_ptr<Node> Parser::parse_variable(shared_ptr<Node> var) {
    log.log_msg("parse_variable", 0);
    shared_ptr<Node> variable = make_shared<Node>("variable", map<string, string>{{"state", "false"}}, var, var->pos);
    if (current_token == Token("symbol", "[")) {
        variable->value["state"] = "index";
        get_token();
        variable->children.push_back(parse_expression());
        if (current_token != Token("symbol", "]"))
            parser_error("Expected ']', not " + current_token.toString());
    } else if (current_token == Token("symbol", ".")) {
        variable->value["state"] = "attr";
        get_token();
        if (current_token.type != "identifier")
            parser_error("Expected identifier, not " + current_token.toString());
        variable->children.push_back(make_shared<Node>("name", map<string, string>{{"name", current_token.value}}, current_token.pos));
    } else
        parser_error("Expected '[' or '.', not " + current_token.toString());
    while (next_token() == Tokens("symbol", {".", "[", "<", "("})) {
        get_token();
        if (current_token == Tokens("symbol", {".", "["}))
            variable = parse_variable(variable);
        else {
            if (current_token == Token("symbol", "<") && !isCall()) {
                rollback_token();
                log.log_msg("parse_variable", 1);
                return variable;
            }
            variable = parse_call(variable);
        }
    }
    log.log_msg("parse_variable", 1);
    return variable;
}

shared_ptr<Node> Parser::parse_use_generic() {
    log.log_msg("parse_use_generic", 0);
    shared_ptr<Node> generic = make_shared<Node>("use_generic", current_token.pos);
    if (current_token == Token("symbol", "<")) {
        do {
            get_token();
            generic->children.push_back(parse_type());
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
        get_token();
    }
    log.log_msg("parse_use_generic", 1);
    return generic;
}

shared_ptr<Node> Parser::parse_declare_generic() {
    log.log_msg("parse_declare_generic", 0);
    shared_ptr<Node> generic = make_shared<Node>("generic", current_token.pos);
    if (current_token == Token("symbol", "<")) {
        do {
            get_token();
            if (current_token.type != "identifier")
                parser_error("Expected identifier, not " + current_token.toString());
            generic->children.push_back(make_shared<Node>("declare_generic", map<string, string>{{"name", current_token.value}}, current_token.pos));
            get_token();
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ">"))
            parser_error("Expected '>', not " + current_token.toString());
        get_token();
    }
    log.log_msg("parse_declare_generic", 1);
    return generic;
}

shared_ptr<Node> Parser::parse_call(shared_ptr<Node> var) {
    log.log_msg("parse_call", 0);
    shared_ptr<Node> call = make_shared<Node>("call", map<string, string>{}, var, var->pos);
    call->children.push_back(parse_use_generic());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    shared_ptr<Node> args_call = make_shared<Node>("args_call", current_token.pos);
    if (next_token() == Token("symbol", ")")) {
        get_token();
    } else {
        do {
            get_token();
            args_call->children.push_back(parse_expression());
        } while (current_token == Token("symbol", ","));
        if (current_token != Token("symbol", ")"))
            parser_error("Expected ')', not " + current_token.toString());
    }
    call->children.push_back(args_call);
    while (next_token() == Tokens("symbol", {".", "[", "<", "("})) {
        get_token();
        if (current_token == Tokens("symbol", {".", "["}))
            call = parse_variable(call);
        else {
            if (current_token == Token("symbol", "<") && !isCall()) {
                rollback_token();
                log.log_msg("parse_call", 1);
                return call;
            }
            call = parse_call(call);
        }
    }
    log.log_msg("parse_call", 1);
    return call;
}

shared_ptr<Node> Parser::parse_function() {
    log.log_msg("parse_function", 0);
    shared_ptr<Node> func = make_shared<Node>("func", current_token.pos);
    get_token();
    if (current_token == Token("keyword", "const")) {
        func->value["const"] = "true";
        get_token();
    } else
        func->value["const"] = "false";
    func->children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    func->value["name"] = current_token.value;
    get_token();
    func->children.push_back(parse_declare_generic());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    shared_ptr<Node> args_declare = make_shared<Node>("args_declare", current_token.pos);
    for (shared_ptr<Node> i : parse_declare_args())
        args_declare->children.push_back(i);
    func->children.push_back(args_declare);
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    func->children.push_back(parse_statements());
    log.log_msg("parse_function", 1);
    return func;
}

shared_ptr<Node> Parser::parse_class() {
    log.log_msg("parse_class", 0);
    shared_ptr<Node> class_node = make_shared<Node>("class", current_token.pos);
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    class_node->value["name"] = current_token.value;
    get_token();
    class_node->children.push_back(parse_declare_generic());
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    while (current_token != Token("symbol", "}")) {
        if (current_token == Tokens("keyword", {"attr", "static"})) {
            for (shared_ptr<Node> i : parse_declare(true, false))
                class_node->children.push_back(i);
        } else if (current_token == Token("keyword", "func"))
            class_node->children.push_back(parse_function());
        else if (current_token == Token("keyword", "method"))
            class_node->children.push_back(parse_method());
        get_token();
    }
    log.log_msg("parse_class", 1);
    return class_node;
}

shared_ptr<Node> Parser::parse_method() {
    log.log_msg("parse_method", 0);
    shared_ptr<Node> method = make_shared<Node>("method", current_token.pos);
    get_token();
    if (current_token == Token("keyword", "op")) {
        method->value["kind"] = "op";
        get_token();
    } else if (current_token == Token("keyword", "public")) {
        method->value["kind"] = "public";
        get_token();
    } else
        method->value["kind"] = "private";

    method->children.push_back(parse_type());
    get_token();
    if (current_token.type == "identifier")
        method->value["name"] = current_token.value;
    else
        parser_error("Expected identifier, not " + current_token.toString());
    get_token();
    method->children.push_back(parse_declare_generic());
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    if (current_token != Token("identifier", "self"))
        parser_error("Excepted keyword 'self', not " + current_token.toString());
    get_token();
    shared_ptr<Node> args_declare = make_shared<Node>("args_declare", current_token.pos);
    if (current_token == Token("symbol", ",")) {
        for (shared_ptr<Node> i : parse_declare_args())
            args_declare->children.push_back(i);
    } else if (current_token != Token("symbol", ")"))
        parser_error("Expected ',' or ')', not " + current_token.toString());
    method->children.push_back(args_declare);
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    method->children.push_back(parse_statements());
    log.log_msg("parse_method", 1);
    return method;
}

vector<shared_ptr<Node>> Parser::parse_declare_args() {
    log.log_msg("parse_declare_args", 0);
    vector<shared_ptr<Node>> args;
    shared_ptr<Node> type;
    if (next_token() == Token("symbol", ")")) {
        get_token();
        log.log_msg("parse_declare_args", 1);
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
        args.push_back(make_shared<Node>("declare_arg", map<string, string>{{"name", current_token.value}, {"tuple", tuple}}, shared_ptr<Node>{type}, current_token.pos));
        get_token();
    } while (current_token == Token("symbol", ","));
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    log.log_msg("parse_declare_args", 1);
    return args;
}

shared_ptr<Node> Parser::parse_arr() {
    log.log_msg("parse_arr", 0);
    shared_ptr<Node> arr = make_shared<Node>("arr", current_token.pos);
    get_token();
    while (current_token != Token("symbol", "]")) {
        arr->children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token != Token("symbol", "]"))
            parser_error("Expected ',' or ']', not " + current_token.toString());
    }
    log.log_msg("parse_arr", 1);
    return arr;
}

shared_ptr<Node> Parser::parse_tuple() {
    log.log_msg("parse_tuple", 0);
    shared_ptr<Node> tuple = make_shared<Node>("tuple", current_token.pos);
    get_token();
    while (current_token != Token("symbol", ")")) {
        tuple->children.push_back(parse_expression());
        if (current_token == Token("symbol", ","))
            get_token();
        else if (current_token == Token("symbol", ")"))
            break;
        else
            parser_error("Expected ',' or ')', not " + current_token.toString());
    }
    if (tuple->children.size() == 1) {
        log.log_msg("parse_tuple", 1);
        return tuple->children[0];
    }
    tuple->value["length"] = to_string(tuple->children.size());
    log.log_msg("parse_tuple", 1);
    return tuple;
}

shared_ptr<Node> Parser::parse_dict() {
    log.log_msg("parse_dict", 0);
    shared_ptr<Node> dict = make_shared<Node>("dict", current_token.pos);
    get_token();
    while (current_token != Token("symbol", "}")) {
        dict->children.push_back(parse_expression());
        if (current_token == Token("symbol", ":")) {
            get_token();
            dict->children.push_back(parse_expression());
        } else
            parser_error("Expected ':', not " + current_token.toString());
        if (current_token == Token("symbol", "}"))
            break;
        else if (current_token != Token("symbol", ","))
            parser_error("Expected ',' or '}', not " + current_token.toString());
    }
    log.log_msg("parse_dict", 1);
    return dict;
}

shared_ptr<Node> Parser::parse_statements() {
    log.log_msg("parse_statements", 0);
    shared_ptr<Node> statements = make_shared<Node>("statements", current_token.pos);
    while (current_token != Token("symbol", "}")) {
        if (current_token == Token("keyword", "if"))
            statements->children.push_back(parse_if());
        else if (current_token == Token("keyword", "for"))
            statements->children.push_back(parse_for());
        else if (current_token == Token("keyword", "while"))
            statements->children.push_back(parse_while());
        else if (current_token == Token("keyword", "break"))
            statements->children.push_back(parse_break());
        else if (current_token == Token("keyword", "return"))
            statements->children.push_back(parse_return());
        else if (current_token == Token("keyword", "continue")) {
            statements->children.push_back(make_shared<Node>("continue", current_token.pos));
            get_token();
            if (current_token != Token("symbol", ";"))
                parser_error("Expected ';', not " + current_token.toString());
        } else if (current_token == Tokens("keyword", {"var", "const"})) {
            for (shared_ptr<Node> i : parse_declare(false, false))
                statements->children.push_back(i);
        } else if (current_token == Tokens("keyword", {"attr", "static"})) {
            for (shared_ptr<Node> i : parse_declare(true, false))
                statements->children.push_back(i);
        } else if (is_term(current_token)) {
            statements->children.push_back(parse_expression());
            if (current_token != Token("symbol", ";"))
                parser_error("Expected ';', not " + current_token.toString());
        } else if (current_token != Token("keyword", "pass"))
            parser_error("Expected statement, not " + current_token.toString());
        get_token();
    }
    log.log_msg("parse_statements", 1);
    return statements;
}

shared_ptr<Node> Parser::parse_if() {
    log.log_msg("parse_if", 0);
    shared_ptr<Node> if_node = make_shared<Node>("if", current_token.pos);
    get_token();
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    if_node->children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    if_node->children.push_back(parse_statements());
    int n = 0;
    while (next_token() == Token("keyword", "elif")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "("))
            parser_error("Expected '(', not " + current_token.toString());
        get_token();
        if_node->children.push_back(parse_expression());
        if (current_token != Token("symbol", ")"))
            parser_error("Expected ')', not " + current_token.toString());
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        if_node->children.push_back(parse_statements());
        n++;
    }
    if_node->value["elif_n"] = to_string(n);
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        if_node->children.push_back(parse_statements());
        if_node->value["else"] = "true";
    } else
        if_node->value["else"] = "false";
    log.log_msg("parse_if", 1);
    return if_node;
}

shared_ptr<Node> Parser::parse_for() {
    log.log_msg("parse_for", 0);
    shared_ptr<Node> for_node = make_shared<Node>("for", current_token.pos);
    get_token();
    if (current_token.type == "identifier") {
        for_node->value["label"] = current_token.value;
        get_token();
    } else
        for_node->value["label"] = "for";
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    for_node->children.push_back(parse_type());
    get_token();
    if (current_token.type != "identifier")
        parser_error("Expected identifier, not " + current_token.toString());
    for_node->value["name"] = current_token.value;
    get_token();
    if (current_token != Token("keyword", "in"))
        parser_error("Expected 'in', not " + current_token.toString());
    get_token();
    for_node->children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    for_node->children.push_back(parse_statements());
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        for_node->children.push_back(parse_statements());
        for_node->value["else"] = "true";
    } else
        for_node->value["else"] = "false";
    log.log_msg("parse_for", 1);
    return for_node;
}

shared_ptr<Node> Parser::parse_while() {
    log.log_msg("parse_while", 0);
    shared_ptr<Node> while_node = make_shared<Node>("while", current_token.pos);
    get_token();
    if (current_token.type == "identifier") {
        while_node->value["label"] = current_token.value;
        get_token();
    } else
        while_node->value["label"] = "while";
    if (current_token != Token("symbol", "("))
        parser_error("Expected '(', not " + current_token.toString());
    get_token();
    while_node->children.push_back(parse_expression());
    if (current_token != Token("symbol", ")"))
        parser_error("Expected ')', not " + current_token.toString());
    get_token();
    if (current_token != Token("symbol", "{"))
        parser_error("Expected '{', not " + current_token.toString());
    get_token();
    while_node->children.push_back(parse_statements());
    if (next_token() == Token("keyword", "else")) {
        get_token();
        get_token();
        if (current_token != Token("symbol", "{"))
            parser_error("Expected '{', not " + current_token.toString());
        get_token();
        while_node->children.push_back(parse_statements());
        while_node->value["else"] = "true";
    } else
        while_node->value["else"] = "false";
    log.log_msg("parse_while", 1);
    return while_node;
}

shared_ptr<Node> Parser::parse_break() {
    log.log_msg("parse_break", 0);
    shared_ptr<Node> break_node = make_shared<Node>("break", current_token.pos);
    get_token();
    if (current_token.type == "identifier") {
        break_node->value["label"] = current_token.value;
        get_token();
    } else
        break_node->value["label"] = "break";
    if (current_token != Token("symbol", ";"))
        parser_error("Expected ';', not " + current_token.toString());
    log.log_msg("parse_break", 1);
    return break_node;
}

shared_ptr<Node> Parser::parse_return() {
    log.log_msg("parse_return", 0);
    shared_ptr<Node> return_node = make_shared<Node>("return", current_token.pos);
    get_token();
    if (current_token == Token("symbol", ";"))
        return return_node;
    else {
        return_node->children.push_back(parse_expression());
        if (current_token != Token("symbol", ";"))
            parser_error("Expected ';', not " + current_token.toString());
    }
    log.log_msg("parse_return", 1);
    return return_node;
}
