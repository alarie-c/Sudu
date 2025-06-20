#ifndef LEXER_H
#define LEXER_H
#define INIT_CAPACITY 1024
#define PRINT_SPAN_INTERNALS true
#include "common.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    TOK_EOF = 0,
    TOK_ILLEGAL,
    TOK_RPAREN,
    TOK_LPAREN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_SYMBOL,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_FUNCTION,
    TOK_LET,
    TOK_NEWLINE,
} Token_Kind;

static const char *TOKEN_KIND_NAMES[] = {
    "EOF",
    "ILLEGAL",
    "RPAREN",
    "LPAREN",
    "PLUS",
    "MINUS",
    "STAR",
    "SLASH",
    "PERCENT",
    "SYMBOL",
    "INTEGER",
    "FLOAT",
    "FUNCTION",
    "LET",
    "NEWLINE",
};

typedef struct {
    Token_Kind kind;
    Span span;
    size_t x; /* column */
    size_t y; /* line */
} Token;

void Print_Token(const char *src, Token const *self);

typedef struct {
    const char *src;
    size_t len;
    size_t pos;
    size_t x;
    size_t y;
} Lexer;

Lexer *Init_Lexer(const char *src);
Token Next_Token(Lexer *self);
void Free_Lexer(Lexer *self);

/* Tests */
void Test_Lexer();

#endif // LEXER_H