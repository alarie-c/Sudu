#ifndef LEXER_H
#define LEXER_H
#define INIT_TOKEN_CAPACITY 1024
#include "util/tests.h"
#include "util/common.h"
#include <stddef.h>
#include <stdbool.h>

List Tokenize(const char *src);

/* Lexer Tests */
void Test_Lexer(Test_Info *info);

#endif // LEXER_H
