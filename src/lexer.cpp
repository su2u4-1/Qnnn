#include "../include/qlib.h"

vector<shared_ptr<Token>> lexer(const vector<string>& source_code, const fs::path& file_name) {
    vector<shared_ptr<Token>> tokens;
    string state = "", content = "";
    char p = ' ', pp = ' ';
    pair<int, int> pos = make_pair(-1, -1);
    for (int i = 0; i < source_code.size(); i++) {
        string line = source_code[i];
        for (int j = 0; j < line.size(); j++) {
            char c = line[j];
            p = pp;
            pp = c;
            if (state == "comment_1") {
                content += c;
                if (p == '*' && c == '/') {
                    tokens.push_back(make_shared<Token>("comment", content, file_name, pos));
                    state = "";
                    content = "";
                }
                continue;
            }
            if (state == "+" || state == "%") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", string(1, p) + c, file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", string(1, p), file_name, make_pair(i + 1, j - 1)));
            } else if (state == "-") {
                state = "";
                if (isdigit(c)) {
                    state = "integer";
                    content = {p, c};
                    pos = {i + 1, j - 1};
                    continue;
                } else if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "-=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "-", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "*") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "*=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else if (c == '*') {
                    tokens.push_back(make_shared<Token>("symbol", "**", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "*", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "/") {
                state = "";
                if (c == '/') {
                    state = "comment_0";
                    content = "//";
                    pos = {i + 1, j - 1};
                    continue;
                } else if (c == '*') {
                    state = "comment_1";
                    content = "/*";
                    pos = {i + 1, j - 1};
                    continue;
                } else if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "/=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "/", file_name, make_pair(i + 1, j - 1)));
            } else if (state == ">") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", ">=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else if (c == '>') {
                    tokens.push_back(make_shared<Token>("symbol", ">>", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", ">", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "<") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "<=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else if (c == '<') {
                    tokens.push_back(make_shared<Token>("symbol", "<<", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "<", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "=") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "==", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "=", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "!") {
                state = "";
                if (c == '=') {
                    tokens.push_back(make_shared<Token>("symbol", "!=", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "!", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "&") {
                state = "";
                if (c == '&') {
                    tokens.push_back(make_shared<Token>("symbol", "&&", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "&", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "|") {
                state = "";
                if (c == '|') {
                    tokens.push_back(make_shared<Token>("symbol", "||", file_name, make_pair(i + 1, j - 1)));
                    continue;
                } else
                    tokens.push_back(make_shared<Token>("symbol", "|", file_name, make_pair(i + 1, j - 1)));
            } else if (state == "char") {
                if (c != '\'') {
                    if (content == "")
                        content = c;
                    else
                        error("Character constant too long", file_name, {i + 1, j}, line);
                } else {
                    if (content.size() == 1) {
                        tokens.push_back(make_shared<Token>("char", content, file_name, make_pair(i + 1, j)));
                        state = "";
                        content = "";
                    } else
                        error("Character constant too long or too short", file_name, {i + 1, j}, line);
                }
                continue;
            } else if (state == "string") {
                if (c == '"') {
                    tokens.push_back(make_shared<Token>("str", content, file_name, pos));
                    state = "";
                    content = "";
                } else
                    content += c;
                continue;
            } else if (state == "integer") {
                if (isdigit(c)) {
                    content += c;
                    continue;
                } else if (c == '.') {
                    state = "float";
                    content += c;
                    continue;
                } else {
                    tokens.push_back(make_shared<Token>("int", content, file_name, pos));
                    state = "";
                    content = "";
                }
            } else if (state == "float") {
                if (isdigit(c)) {
                    content += c;
                    continue;
                } else {
                    tokens.push_back(make_shared<Token>("float", content, file_name, pos));
                    state = "";
                    content = "";
                }
            } else if (state == "comment_0") {
                if (c != '\n')
                    content += c;
                else {
                    tokens.push_back(make_shared<Token>("comment", content, file_name, pos));
                    state = "";
                    content = "";
                }
                continue;
            } else if (state == "identifier") {
                if (isalnum(c) || c == '_') {
                    content += c;
                    continue;
                } else {
                    if (is_keyword(content))
                        tokens.push_back(make_shared<Token>("keyword", content, file_name, pos));
                    else
                        tokens.push_back(make_shared<Token>("identifier", content, file_name, pos));
                    state = "";
                    content = "";
                }
            }

            if (is_symbol(c)) {
                if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '>' || c == '=' || c == '<' || c == '!' || c == '&' || c == '|')
                    state = string(1, c);
                else
                    tokens.push_back(make_shared<Token>("symbol", string(1, c), file_name, make_pair(i + 1, j)));
            } else if (c == '\'')
                state = "char";
            else if (c == '"') {
                state = "string";
                pos = {i + 1, j + 1};
            } else if (isdigit(c)) {
                state = "integer";
                pos = {i + 1, j};
                content = c;
            } else if (c == '#') {
                state = "comment_0";
                content = "#";
                pos = {i + 1, j};
            } else if (isalpha(c) || c == '_') {
                state = "identifier";
                pos = {i + 1, j};
                content = c;
            } else {
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\0' && c != '\v' && c != '\f')
                    error("Invalid character " + string(1, c), file_name, {i + 1, j}, line);
            }
        }
    }
    return tokens;
}
