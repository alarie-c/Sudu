#ifndef PARSER_H
#define PARSER_H
#define INIT_AST_CAPACITY 512
#define INIT_PROGRAM_CAPACITY 64
#include "common.h"
#include "lexer.h"

/// @brief Enum constants for the different types of nodes in the AST,
/// corresponding with what is stored in the Ast_Node union type
typedef enum {
    AST_PROGRAM = 0,
    AST_BINARY_EXPR,
    AST_FLOAT,
    AST_INTEGER,
} Node_Type;

/// @brief Top-level program node that holds every declaration in the file.
/// Implemented as a dynamically resizable array of size_t
/// (ids corresponding to nodes in the map)
typedef struct {
    size_t *ids;
    size_t size;
    size_t capacity;
} Ast_Program;

/// @brief Holds ever operator for binary expressions
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
} Ast_Binary_Op;

typedef struct {
    Ast_Binary_Op op;
    size_t lhs;
    size_t rhs;
} Ast_Binary_Expr;

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member
typedef struct {
    Node_Type type;
    union {
        Ast_Program data_program;
        Ast_Binary_Expr data_binary_expr;
        float data_float;
        long int data_integer;
    };
    Span span;
} Ast_Node;

/// @brief Just holds all of the nodes in a contiguous dynamically resiable array of Ast_Node
typedef struct {
    Ast_Node *nodes;
    size_t size;
    size_t capacity;
} Node_Map;

void Free_Node_Map(Node_Map *map);
void Print_Node(Node_Map *map, size_t id, int i);

typedef struct {
    const char *src;
    Lexer *lexer;
    Node_Map *map;
} Parser;

Parser *Init_Parser(const char *src);
void Free_Parser(Parser *self);
void Parse(Parser *self);

#endif // PARSER_H
