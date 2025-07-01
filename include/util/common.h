#ifndef COMMON_H
#define COMMON_H
#define WITH_ESCAPES true
#define NO_ESCAPES false
#define LIST_GROWTH_FACTOR 2
#define CHECK_LIST_COMPAT_TYPE(list, type) ((list)->size == sizeof(type))
#define PRINT_SPAN_INTERNALS 1
#include "util/tests.h"
#include <stddef.h>
#include <stdbool.h>

//-------------------------------------------------------------------------------//
// dynamic array
//-------------------------------------------------------------------------------//

typedef struct
{
    void *data;
    size_t size;
    size_t count;
    size_t capacity;
} List;

List List_New(size_t size, size_t init_capacity);
void *List_Get(List *self, size_t n);
void List_Add(List *self, void* element);
void List_Free(List *self);

//-------------------------------------------------------------------------------//
// span & lexemes
//-------------------------------------------------------------------------------//

typedef struct
{
    size_t pos;
    size_t len;
} Span;

size_t Lexeme_Buffer_Len(Span const *span);
void Get_Lexeme(char *buf, size_t buf_size, const char *src, Span const *span, bool escapes);
bool Cmp_Lexeme(const char *src, const Span *span, const char *lit);
void Remove_Underbars(char *str);

//-------------------------------------------------------------------------------//
// tokens
//-------------------------------------------------------------------------------//

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

//-------------------------------------------------------------------------------//
// Tests
//-------------------------------------------------------------------------------//

void Test_List(Test_Info *info);

#endif // COMMON_H
