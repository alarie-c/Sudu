#ifndef PARSER_H
#define PARSER_H
#define INIT_AST_CAPACITY 512
#include "common.h"
#include "lexer.h"
#include "tests.h"
#include "errors.h"
#include "ast.h"
#include <stdbool.h>

typedef struct
{
    const char *src;
   
    /* sub structures */

    Lexer *lexer;
    Node_Map *map;
    Error_Collection *errors;

    /* token caches */

    Token current_tk;
    Token peeked_tk;

    /* helper booleans */

    bool is_peeked;
    bool is_at_end;
} Parser;

Parser *Init_Parser(const char *src);
void Free_Parser(Parser *self);
void Parse(Parser *self);

void Test_Binary_Expression(Test_Info *info);
void Test_Grouping_Expression(Test_Info *info);

#endif // PARSER_H

