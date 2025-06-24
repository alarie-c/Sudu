#ifndef PARSER_H
#define PARSER_H
#define INIT_AST_CAPACITY 512
#define INIT_PROGRAM_CAPACITY 64
#include "common.h"
#include "lexer.h"
#include "tests.h"
#include "errors.h"
#include <stdbool.h>

/// @brief Enum constants for the different types of nodes in the AST,
/// corresponding with what is stored in the Ast_Node union type
typedef enum
{
    AST_PROGRAM = 0,
    AST_BINARY_EXPR,
    AST_FLOAT,
    AST_INTEGER,
    AST_GROUPING,
} Node_Type;

/// @brief Top-level program node that holds every declaration in the file.
/// Implemented as a dynamically resizable array of size_t
/// (ids corresponding to nodes in the map)
typedef struct
{
    size_t *ids;
    size_t size;
    size_t capacity;
} Ast_Program;

/// @brief Holds ever operator for binary expressions
typedef enum
{
    OP_NONE = 0,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
} Ast_Op;

static int AST_BINARY_INFIX[NUM_TOKEN_KINDS] = {
    [TOK_PLUS] = OP_ADD,
    [TOK_MINUS] = OP_SUB,
    [TOK_STAR] = OP_MUL,
    [TOK_SLASH] = OP_DIV,
    [TOK_PERCENT] = OP_MOD,
};

static const char *AST_OP_NAMES[] = {
    [OP_NONE] = "None",
    [OP_ADD] = "+",
    [OP_SUB] = "-",
    [OP_MUL] = "*",
    [OP_DIV] = "/",
    [OP_MOD] = "%",
};

typedef struct
{
    Ast_Op op;
    size_t lhs;
    size_t rhs;
} Ast_Binary_Expr;

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member
typedef struct
{
    Node_Type type;
    union {
        Ast_Program v_program;
        Ast_Binary_Expr v_binary_expr;
        float v_float;
        long int v_integer;
        size_t v_inner;
    };
    Span span;
} Ast_Node;

/// @brief Just holds all of the nodes in a contiguous dynamically resiable array of Ast_Node
typedef struct
{
    Ast_Node *nodes;
    size_t size;
    size_t capacity;
} Node_Map;

void Free_Node_Map(Node_Map *map);
void Print_Node(Node_Map *map, size_t id, int i);

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

