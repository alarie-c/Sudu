#ifndef LEXER_H
#define LEXER_H
#define INIT_TOKEN_CAPACITY 512
#include "util/common.h"
#include "util/tests.h"
#include <stdbool.h>

typedef struct _Tokens
{
    List tokens;
    bool valid;
} Tokens;

/// @brief Tokenizes the source input and produces a stream of tokens.
/// @param src source code as a string.
/// @param errors a pointer to the errors buffer.
/// @return struct containing a `List<Token>` and a `bool` for success/failure.
Tokens Tokenize(const char *src, List *errors);

/* Tests */
void Test_Lexer(Test_Info *info);
void Test_Literals(Test_Info *info);

#endif // LEXER_H
