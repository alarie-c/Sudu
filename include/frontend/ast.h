#ifndef AST_H
#define AST_H

#include "util/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef size_t Node_Idx;

//===============================================================================//
// AST NODE HELPER ENUMS
//===============================================================================//

/// @brief Enum constants for the different types of nodes in the AST,
/// corresponding with what is stored in the Ast_Node union type.
typedef enum _Node_Kind
{
    NODE_ROOT = 0,
    NODE_INTEGER,
    NODE_FLOAT,
    NODE_SYMBOL,
    NODE_GROUPING,
    NODE_LIST,
    NODE_CALL,
    NODE_BINARY,
    NODE_VARIABLE,
    NODE_ASSIGNMENT,
} Node_Kind;

//===============================================================================//
// COMPLEX NODE STRUCTS
//===============================================================================//

typedef struct _Ast_Variable_Decl
{
    Node_Idx symbol;
    Node_Idx initializer;
    bool mutability;
} Ast_Variable_Decl;

typedef struct _Ast_Binary_Expr
{
    Ast_Op_Kind op;
    Node_Idx lhs;
    Node_Idx rhs;
} Ast_Binary_Expr;

typedef struct _Ast_Assign_Expr
{
    Ast_Op_Kind op;
    Node_Idx name;
    Node_Idx value;
} Ast_Assign_Expr;

typedef struct _Ast_Call_Expr
{
    Node_Idx symbol;
    Node_Idx args;
} Ast_Call_Expr;

typedef struct _Ast_List
{
    Node_Idx *nodes;
    size_t count;
    size_t capacity;
} Ast_List;

Ast_List Ast_List_New(size_t init_capacity);
void Ast_List_Push(Ast_List *self, Node_Idx node);

//===============================================================================//
// AST NODES
//===============================================================================//

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member.
typedef struct _Ast_Node
{
    Node_Kind type;
    union {
        List v_root;
        Ast_Variable_Decl v_variable_decl;
        Ast_Binary_Expr v_binary_expr;
        Ast_Assign_Expr v_assign_expr;
        Ast_Call_Expr v_call_expr;
        Ast_List v_list;
        char *v_symbol;
        float v_float;
        int32_t v_integer;
        Node_Idx v_inner;
    };
    Span span;
} Ast_Node;

/// @brief Pushes a node to the node map and returns it's ID/index.
/// @param nodes the node map.
/// @param node the node instance itself.
/// @return the index of the node in the map.
Node_Idx Push_And_Get_Id(List *nodes, Ast_Node node);

/// @brief Simple prints a node using a recursive method..
/// @param nodes the node map.
/// @param id the index of the node in the map.
/// @param i number of spaces worth of indentation.
void Print_Node(List *nodes, Node_Idx id, int i);

/// @brief Frees a node, if neccesary. Really only needed.
/// for nodes that own dynamically allocated memory, this does not
/// free the actual node itself.
/// @param self the node.
void Node_Free(Ast_Node *self);

//===============================================================================//
// NODE BUILDER HELPER FUNCTIONS
//===============================================================================//

/// @brief Creates a node from the given type and sets the span, leaving everything
/// else to be zero'd out.
/// @param type the node type.
/// @param span the span instance.
/// @return a zero'd out node with `type` and `span` members set.
Ast_Node Node_Build(Node_Kind type, Span span);

/// @brief Creates an integer node from the value provided.
/// @param nodes the node map to be inserted into.
/// @param span the span of the node.
/// @param value the integer value.
/// @return a node of type AST_INTEGER.
Node_Idx Node_Build_Integer(List *nodes, Span const span, int32_t value);

/// @brief Creates an float node from the value provided.
/// @param nodes the node map to be inserted into.
/// @param span the span of the node.
/// @param value the float value.
/// @return a node of type AST_FLOAT.
Node_Idx Node_Build_Float(List *nodes, Span const span, float value);

/// @brief Creates a grouping node from the inner node ID provided.
/// @param nodes the node map to be inserted into   .
/// @param span the span of the node.
/// @param inner the ID/index of the node inside the grouping.
/// @return a node of type AST_GROUPING.
Node_Idx Node_Build_Grouping(List *nodes, Span const span, Node_Idx inner);

/// @brief Creates a symbol node from the name provided.
/// @param nodes the node map to be inserted into  .
/// @param span the span of the node.
/// @param src source code to extract the lexeme from.
/// @return a node of type AST_SYMBOL.
Node_Idx Node_Build_Symbol(List *nodes, Span const span, const char *src);

//===============================================================================//
// ABSTRACT SYNTAX TREE
//===============================================================================//

/// @brief Contains the relevant information for the abstract syntax tree, including the
/// node map for the nodes to be stored contiguously in memory, along with the root node.
typedef struct
{
    Ast_Node root_node;
    List node_map;
} Abstract_Syntax_Tree;

#endif // AST_H
