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
    AST_ASSIGN_EXPR,
    AST_GROUPING,
    AST_SYMBOL,
    AST_INTEGER,
    AST_FLOAT,
} Node_Type;

/// @brief Holds ever operator for binary expressions
typedef enum
{
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

typedef struct
{
    Ast_Op op;
    size_t name;
    size_t value;
} Ast_Assign_Expr;

//-------------------------------------------------------------------------------//
// node
//-------------------------------------------------------------------------------//

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member
typedef struct
{
    Node_Type type;
    union {
        List v_root;
        Ast_Binary_Expr v_binary_expr;
        Ast_Assign_Expr v_assign_expr;
        char *v_symbol;
        float v_float;
        int32_t v_integer;
        size_t v_inner;
    };
    Span span;
} Ast_Node;

size_t Push_And_Get_Id(List *nodes, Ast_Node node);
void Print_Node(List *nodes, size_t id, int i);
void Node_Free(Ast_Node *self);

//-------------------------------------------------------------------------------//
// node builders
//-------------------------------------------------------------------------------//

Ast_Node Node_Build(Node_Type type, Span span);
size_t Node_Build_Integer(List *nodes, Span const span, int32_t value);
size_t Node_Build_Float(List *nodes, Span const span, float value);
size_t Node_Build_Grouping(List *nodes, Span const span, size_t inner);
size_t Node_Build_Symbol(List *nodes, Span const span, const char *src);

//-------------------------------------------------------------------------------//
// static maps
//-------------------------------------------------------------------------------//

static inline int Get_Binary_Infix_Operator(Token_Kind kind)
{
    // printf("Getting Binary Infix: %i\n", kind);
    switch (kind)
    {
        case TOK_PLUS: return OP_ADD;
        case TOK_MINUS: return OP_SUB;
        case TOK_STAR: return OP_MUL;
        case TOK_SLASH: return OP_DIV;
        case TOK_PERCENT: return OP_MOD;
        default: return -1;
    }
}

static inline int Get_Assignment_Infix_Operator(Token_Kind kind)
{
    // printf("Getting Assignment Infix: %i\n", kind);
    switch (kind)
    {
        case TOK_EQ: return OP_EQ;
        case TOK_PLUS_EQ: return OP_PLUS_EQ;
        default: return -1;
    }
}

static const char *AST_OP_NAMES[] = {
    [OP_ADD]     = "+",
    [OP_SUB]     = "-",
    [OP_MUL]     = "*",
    [OP_DIV]     = "/",
    [OP_MOD]     = "%",
    [OP_EQ]      = "=",
    [OP_PLUS_EQ] = "+=",
};

#endif // AST_H