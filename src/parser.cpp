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

void Parser::error(string message, pair<int, int> location) {
}

Node Parser::parse() {
}

Node Parser::parse_import() {
}

Node Parser::parse_declare(int state) {
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
