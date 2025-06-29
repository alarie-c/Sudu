#ifndef AST_H
#define AST_H
#define INIT_PROGRAM_CAPACITY 64
#include "frontend/lexer.h"
#include "util/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// @brief Enum constants for the different types of nodes in the AST,
/// corresponding with what is stored in the Ast_Node union type
typedef enum
{
    AST_PROGRAM = 0,
    AST_BINARY_EXPR,
    AST_ASSIGNMENT,
    AST_GROUPING,
    AST_INTEGER,
    AST_FLOAT,
} Node_Type;

/// @brief Holds ever operator for binary expressions
typedef enum
{
    OP_NONE = 0,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    /* assignment */

    OP_EQ,
    OP_PLUS_EQ,
} Ast_Op;

//-------------------------------------------------------------------------------//
// complex node structures
//-------------------------------------------------------------------------------//

typedef struct
{
    Ast_Op op;
    size_t lhs;
    size_t rhs;
} Ast_Binary_Expr;

//-------------------------------------------------------------------------------//
// node
//-------------------------------------------------------------------------------//

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member
typedef struct
{
    Node_Type type;
    union {
        List v_program;
        Ast_Binary_Expr v_binary_expr;
        float v_float;
        int32_t v_integer;
        size_t v_inner;
    };
    Span span;
} Ast_Node;

size_t Push_And_Get_Id(List *nodes, Ast_Node node);
void Print_Node(List *nodes, size_t id, int i);

//-------------------------------------------------------------------------------//
// node builders
//-------------------------------------------------------------------------------//

Ast_Node Node_Build(Node_Type type, Span span);
size_t Node_Build_Integer(List *nodes, Span const span, int32_t value);
size_t Node_Build_Float(List *nodes, Span const span, float value);
size_t Node_Build_Grouping(List *nodes, Span const span, size_t inner);

//-------------------------------------------------------------------------------//
// static maps
//-------------------------------------------------------------------------------//

static int AST_BINARY_INFIX[NUM_TOKEN_KINDS] = {
    [TOK_PLUS] = OP_ADD,
    [TOK_MINUS] = OP_SUB,
    [TOK_STAR] = OP_MUL,
    [TOK_SLASH] = OP_DIV,
    [TOK_PERCENT] = OP_MOD,

    [TOK_EQ]      = OP_EQ,
    [TOK_PLUS_EQ] = OP_PLUS_EQ,
};

static const char *AST_OP_NAMES[] = {
    [OP_NONE]    = "None",
    [OP_ADD]     = "+",
    [OP_SUB]     = "-",
    [OP_MUL]     = "*",
    [OP_DIV]     = "/",
    [OP_MOD]     = "%",
    [OP_EQ]      = "=",
    [OP_PLUS_EQ] = "+=",
};

#endif // AST_H