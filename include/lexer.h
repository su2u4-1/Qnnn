#ifndef LEXER_H
#define LEXER_H

#include "qlib.h"

vector<shared_ptr<Token>> lexer(const vector<string>& source_code, const string& file_name);

#endif  // LEXER_H
