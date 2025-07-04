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
/// @param element the item to add, deep copy into
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

#define TOKEN_LIST \
    X(TOK_ILLEGAL,         "ILLEGAL") \
    X(TOK_OPEN_PAREN,      "OPEN_PAREN") \
    X(TOK_CLOSE_PAREN,     "CLOSE_PAREN") \
    X(TOK_EQUALS,          "EQUALS") \
    X(TOK_PLUS_EQUALS,     "PLUS_EQUALS") \
    X(TOK_PLUS,            "PLUS") \
    X(TOK_MINUS,           "MINUS") \
    X(TOK_STAR,            "STAR") \
    X(TOK_SLASH,           "SLASH") \
    X(TOK_PERCENT,         "PERCENT") \
    X(TOK_COMMA,           "COMMA") \
    X(TOK_SEMICOLON,       "SEMICOLON") \
    X(TOK_SYMBOL_LITERAL,  "SYMBOL_LITERAL") \
    X(TOK_INTEGER_LITERAL, "INTEGER_LITERAL") \
    X(TOK_STRING_LITERAL,  "STRING_LITERAL") \
    X(TOK_FLOAT_LITERAL,   "FLOAT_LITERAL") \
    X(TOK_VAR,             "VAR") \
    X(TOK_CONST,           "CONST") \
    X(TOK_NEWLINE,         "NEWLINE") \
    X(TOK_EOF,             "EOF")

typedef enum _Token_Kind
{
    #define X(name, str) name,
    TOKEN_LIST
    #undef X
} Token_Kind;

static const char *TOKEN_KIND_NAMES[] = {
    #define X(name, str) [name] = str,
    TOKEN_LIST
    #undef x
};

typedef struct _Token
{
    Token_Kind kind;
    Span span;
    size_t x;
    size_t y;
} Token;

/// @brief Prints a token, including all of it's span information
void Print_Token(const char *src, Token const *self);

//===============================================================================//
// OPERATORS
//===============================================================================//

#define OP_FLAG_BINARY (1 << 0)
#define OP_FLAG_UNARY  (1 << 1)
#define OP_FLAG_ASSIGN (1 << 2)

#define OPERATOR_LIST \
    X(OP_ADD,        TOK_PLUS,        "ADD",        OP_FLAG_BINARY) \
    X(OP_SUB,        TOK_MINUS,       "SUBTRACT",   OP_FLAG_BINARY  | OP_FLAG_UNARY) \
    X(OP_MUL,        TOK_STAR,        "MULTIPLY",   OP_FLAG_BINARY) \
    X(OP_DIV,        TOK_SLASH,       "DIVIDE",     OP_FLAG_BINARY) \
    X(OP_MOD,        TOK_PERCENT,     "MODULO",     OP_FLAG_BINARY) \
    X(OP_ASSIGN,     TOK_EQUALS,      "ASSIGN",     OP_FLAG_ASSIGN) \
    X(OP_ADD_ASSIGN, TOK_PLUS_EQUALS, "ADD-ASSIGN", OP_FLAG_ASSIGN) 

typedef enum _Operator
{
    #define X(name, token, str, flag) name,
    OPERATOR_LIST
    #undef X
} Operator;

/// @brief Returns the `Operator` pertaining to this token kind that is a binary operator.
inline int Get_Binary_Operator(Token_Kind kind);

/// @brief Returns the `Operator` pertaining to this token kind that is a unary operator.
inline int Get_Unary_Operator(Token_Kind kind);

/// @brief Returns the `Operator` pertaining to this token kind that is an assignment operator.
inline int Get_Assign_Operator(Token_Kind kind);

/// @brief Maps an `Operator` to a `const char*`.
static const char *OPERATOR_NAMES[] = {
    #define X(name, token, str, flag) [name] = str,
    OPERATOR_LIST
    #undef X
};

#endif // COMMON_H
