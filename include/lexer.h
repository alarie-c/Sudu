#ifndef LEXER_H
#define LEXER_H
#define INIT_CAPACITY 1024
#define PRINT_SPAN_INTERNALS true
#include "tests.h"
#include "common.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum
{
    TOK_ILLEGAL = 0,
    TOK_RPAREN,
    TOK_LPAREN,
    TOK_EQ,
    TOK_EQ_EQ,
    TOK_PLUS,
    TOK_PLUS_EQ,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_SYMBOL,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_PROC,
    TOK_LET,
    TOK_MUT,
    TOK_NEWLINE,
    TOK_EOF,
} Token_Kind;
#define NUM_TOKEN_KINDS (TOK_EOF + 1)

static const char *TOKEN_KIND_NAMES[NUM_TOKEN_KINDS] = {
    [TOK_ILLEGAL] = "ILLEGAL",
    [TOK_RPAREN] = "RPAREN",
    [TOK_LPAREN] = "LPAREN",
    [TOK_EQ] = "EQ",
    [TOK_EQ_EQ] = "EQ_EQ",
    [TOK_PLUS] = "PLUS",
    [TOK_PLUS_EQ] = "PLUS_EQ",
    [TOK_MINUS] = "MINUS",
    [TOK_STAR] = "STAR",
    [TOK_SLASH] = "SLASH",
    [TOK_PERCENT] = "PERCENT",
    [TOK_SYMBOL] = "SYMBOL",
    [TOK_INTEGER] = "INTEGER",
    [TOK_FLOAT] = "FLOAT",
    [TOK_PROC] = "PROC",
    [TOK_LET] = "LET",
    [TOK_MUT] = "MUT",
    [TOK_NEWLINE] = "NEWLINE",
    [TOK_EOF] = "EOF"
};

typedef struct
{
    Token_Kind kind;
    Span span;
    size_t x; /* column */
    size_t y; /* line */
} Token;

void Print_Token(const char *src, Token const *self);

typedef struct
{
    const char *src;
    size_t len;
    size_t pos;
    size_t x;
    size_t y;
} Lexer;

Lexer *Init_Lexer(const char *src);
Token Next_Token(Lexer *self);
void Free_Lexer(Lexer *self);

/* Lexer Tests */
void Test_Lexer(Test_Info *info);

#endif // LEXER_H
