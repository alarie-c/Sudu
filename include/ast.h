#ifndef AST_H
#define AST_H
#define INIT_PROGRAM_CAPACITY 64
#include "lexer.h"
#include <stdbool.h>
#include <stddef.h>

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

typedef struct
{
    Ast_Op op;
    size_t lhs;
    size_t rhs;
} Ast_Binary_Expr;

//-------------------------------------------------------------------------------//
// main structures
//-------------------------------------------------------------------------------//

typedef struct Ast_Node Ast_Node;
typedef struct Node_Map Node_Map;

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member
struct Ast_Node
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
};

void Print_Node(Node_Map *map, size_t id, int i);

//-------------------------------------------------------------------------------//
// node builders
//-------------------------------------------------------------------------------//

Ast_Node Node_Build(Node_Type type, Span span);
size_t Node_Build_Integer(Node_Map *map, Span const span, long int value);
size_t Node_Build_Float(Node_Map *map, Span const span, float value);

//-------------------------------------------------------------------------------//
// node map
//-------------------------------------------------------------------------------//

/// @brief Just holds all of the nodes in a contiguous dynamically resiable array of Ast_Node
struct Node_Map
{
    Ast_Node *nodes;
    size_t size;
    size_t capacity;
};

void Free_Node_Map(Node_Map *map);
size_t Node_Map_Push(Node_Map *self, Ast_Node node);
Ast_Node *Node_Map_Get(Node_Map const *self, size_t i);
Node_Map *Node_Map_Init(size_t cap);

//-------------------------------------------------------------------------------//
// maps
//-------------------------------------------------------------------------------//

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

#endif // AST_H