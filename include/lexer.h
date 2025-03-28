#ifndef LEXER_H
#define LEXER_H

#include "qlib.h"

pair<vector<Token>, string> lexer(vector<string> source_code, string file_name);

#endif  // LEXER_H
