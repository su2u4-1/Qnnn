#ifndef PARSER_H
#define PARSER_H

#include "qlib.h"

class Parser {
   public:
    Parser(vector<Token> tokens, string file_name);
    vector<Token> tokens;
    string file_name;
    int index;
    Token current_token;
    Token next_token();
    void get_token();
    Node parse();
    Node parse_import();
    vector<Node> parse_declare();
    Node parse_type();
    Node parse_expression();
    Node parse_term();
    Node parse_variable();
    Node parse_call();
    Node parse_function();
    Node parse_class();
    Node parse_method();
    Node parse_args();
    Node parse_arr();
    Node parse_tuple();
    Node parse_dict();
    Node parse_statement();
    Node parse_if();
    Node parse_for();
    Node parse_while();
    Node parse_break();
    Node parse_return();
};

#endif  // PARSER_H
