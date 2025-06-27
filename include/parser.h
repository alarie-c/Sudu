#ifndef PARSER_H
#define PARSER_H
#define CHECK_NONZERO(node) if ((node) == 0) return 0
#define PRINT_NODE(id) Print_Node(self->nodes, id, 2)
#define PRINT_TOKEN(tk) Print_Token(self->src, &tk)
#include "ast.h"
#include "common.h"
#include "tests.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef struct
{
    const char *src;
    char *path; /* this is strdup'd */
    Lexer *lexer;
    List errors;
    List nodes;
    Token current_tk;
    Token peeked_tk;
    bool is_peeked;
    bool is_at_end;
} Parser;

Parser *Parser_Init(const char *src, const char *path);
void Parser_Free(Parser *self);
void Parse(Parser *self);

/* Tests */
void Test_Binary_Expression(Test_Info *info);
void Test_Grouping_Expression(Test_Info *info);

#endif // PARSER_H