#ifndef COMMON_H
#define COMMON_H
#include "util/tests.h"
#include <stddef.h>
#include <stdbool.h>

//===============================================================================//
// DYNAMIC ARRAY IMPLEMENTATION
//===============================================================================//

#define LIST_GROWTH_FACTOR 2
#define CHECK_LIST_COMPATIBILITY(list, type) ((list)->size == sizeof(type))

typedef struct _List
{
    void *data;
    size_t size;
    size_t count;
    size_t capacity;
} List;

/// @brief Creates a new empty list allocated to the initial capacity.
/// @param size size of the items being stored in this list.
/// @param init_capacity how big to make the array upon allocation.
/// @return An empty list.
List List_New(size_t size, size_t init_capacity);

/// @brief Returns a pointer to the item at `n` position in the list.
/// @param self the list itself.
/// @param n the index of the desired item.
/// @return the pointer to the item, will be `NULL` if out of bounds.
void *List_Get(List *self, size_t n);

/// @brief Push something to the list.
/// @param self the list itself.
/// @param element the item to add, shallow copy into
/// the array, free'd at the end of scope.
void List_Add(List *self, void* element);

/// @brief Free the list and everything in it. Be sure to free any dynamically allocated
/// sub-array members before calling this.
/// @param self the list to free.
void List_Free(List *self);

//===============================================================================//
// SPAN & LEXEME FUNCTIONS
//===============================================================================//

/// @brief Holds an offset+len style position for pointing to source code.
typedef struct _Span
{
    size_t pos;
    size_t len;
} Span;

/// @brief Dynamically allocate and return a lexeme from the given offset+len.
/// @param src the source code the span points to.
/// @param pos the offset into the source code.
/// @param len how much of the source code to copy.
/// @return dynamically allocated `char*`.
char *Get_Lexeme(const char *src, size_t pos, size_t len);

/// @brief Compares whether or not two lexemes are the same
/// @param src the source code the span points to
/// @param span the offset+len to compare
/// @param lit what to compare to
/// @return boolean result
bool Cmp_Lexeme(const char *src, const Span *span, const char *lit);

/// @brief Removes the underbars from a lexeme;
/// @param str lexeme to alter.
void Remove_Underbars(char *str);

//===============================================================================//
// TOKENS
//===============================================================================//

typedef enum _Token_Kind
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

typedef struct _Token
{
    Token_Kind kind;
    Span span;
    size_t x; /* column */
    size_t y; /* line */
} Token;

void Print_Token(const char *src, Token const *self);

#endif // COMMON_H
