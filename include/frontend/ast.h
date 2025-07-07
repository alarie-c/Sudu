#ifndef AST_H
#define AST_H

#include "util/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INIT_NODE_MAP_CAPACITY 256
#define INIT_ROOT_NODE_CAPACITY 64

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
// NODE SUBTYPE STRUCTS
//===============================================================================//

typedef struct _Node_Variable
{
    Node_Idx symbol;
    Node_Idx initializer;
    bool mutability;
} Node_Variable;

typedef struct _Node_Binary
{
    Operator op;
    Node_Idx lhs;
    Node_Idx rhs;
} Node_Binary;

typedef struct _Node_Assignment
{
    Operator op;
    Node_Idx sym;
    Node_Idx val;
} Node_Assignment;

typedef struct _Node_Call
{
    Node_Idx sym;
    Node_Idx args;
} Node_Call;

/// @brief A dynamically resizeable array of Node indices that can be
/// created as a node itself and used to represent things like call args,
/// procedure parameters, class members, etc.
typedef struct _Node_List
{
    Node_Idx *nodes;
    size_t count;
    size_t capacity;
} Node_List;

/// @brief Push a node index to the node list.
/// @param self the node list.
/// @param node the node index.
void Node_List_Add(Node_List *self, Node_Idx node);

//===============================================================================//
// AST NODES
//===============================================================================//

/// @brief The encompassing Ast_Node type for all AST nodes, variation comes in the form of
/// the type member and the union type member.
typedef struct _Node
{
    Node_Kind type;
    Span span;
    union {
        Node_List root;
        Node_Variable variable;
        Node_Binary binary;
        Node_Assignment assignment;
        Node_Call call;
        Node_List list;
        char *symbol_name;
        float float_value;
        int32_t int_value;
        Node_Idx inner_node;
    } data;
} Node;

/// @brief Pushes a node to the node map and returns it's ID/index.
/// @param nodes the node map.
/// @param node the node instance itself.
/// @return the index of the node in the map.
Node_Idx Node_Insert(List *map, Node node);

/// @brief Simple prints a node using a recursive method..
/// @param nodes the node map.
/// @param id the index of the node in the map.
/// @param i number of spaces worth of indentation.
void Node_Print(List *map, Node_Idx id, int i);

/// @brief Frees a node, if neccesary. Really only needed.
/// for nodes that own dynamically allocated memory, this does not
/// free the actual node itself.
/// @param self the node.
void Node_Free(Node *self);

//===============================================================================//
// NODE BUILDER HELPER FUNCTIONS
//===============================================================================//

Node_Idx Make_Node_Integer(List *map, Span const span, int32_t value);
Node_Idx Make_Node_Float(List *map, Span const span, float value);
Node_Idx Make_Node_Grouping(List *map, Span const span, Node_Idx inner);
Node_Idx Make_Node_Symbol(List *map, Span const span, const char *src);
Node_Idx Make_Node_Binary(List *map, Span const span, Node_Idx lhs, Node_Idx rhs, Operator op);
Node_Idx Make_Node_Assignment(List *map, Span const span, Node_Idx sym, Node_Idx val, Operator op);
Node_Idx Make_Node_Variable(List *map, Span const span, Node_Idx sym, Node_Idx initializer, bool mut);
Node_Idx Make_Node_Call(List *map, Span const span, Node_Idx sym, Node_Idx args);
Node_Idx Make_Node_List(List *map, Span const start_span, size_t capacity);

//===============================================================================//
// ABSTRACT SYNTAX TREE
//===============================================================================//

/// @brief Contains the relevant information for the abstract syntax tree, including the
/// node map for the nodes to be stored contiguously in memory, along with the root node.
typedef struct
{
    Node *root_node;
    List node_map;
} AST;

/// @brief Initializes an empty AST with nothing but the root node allocated.
/// @return blank AST.
AST AST_Init();

/// @brief Pushes a node index to the root node of the AST.
/// @param self the AST.
/// @param node_idx index of the node.
void AST_Insert(AST *self, Node_Idx node_idx);

#endif // AST_H
