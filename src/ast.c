#include "ast.h"
#include "tests.h"
#include "common.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#define PRINT_NODE(id) Print_Node(self->map, id, 2)

//-------------------------------------------------------------------------------//
// program node methods
//-------------------------------------------------------------------------------//

struct program_node { Ast_Node node; bool is_valid; };
struct program_node init_program_node(size_t cap)
{
    Ast_Node node = {0};
    
    size_t *ids = malloc(cap * sizeof(size_t));
    if (!ids) return (struct program_node) {node, false};

    Ast_Program program = (Ast_Program) {ids, 0, cap};
    node.type = AST_PROGRAM;
    node.v_program = program;

    return (struct program_node) {node, true};
}

void free_program_node(Ast_Program *self)
{
    if (!self) return;
    free(self->ids);
}

//-------------------------------------------------------------------------------//
// node map methods
//-------------------------------------------------------------------------------//

Node_Map *Node_Map_Init(size_t cap)
{
    Ast_Node *nodes = malloc(cap * sizeof(Ast_Node));
    if (!nodes) return NULL;

    Node_Map *ast = malloc(sizeof(Node_Map));
    if (!ast)
    {
        free(nodes);
        return NULL;
    }

    ast->size = 1;
    ast->capacity = cap;
    ast->nodes = nodes;

    struct program_node program = init_program_node(INIT_PROGRAM_CAPACITY);
    if (!program.is_valid)
    {
        free(nodes);
        return NULL;
    }

    ast->nodes[0] = program.node;
    return ast;
}

size_t Node_Map_Push(Node_Map *self, Ast_Node node)
{
    if (!self) return 0;

    if (self->size >= self->capacity)
    {
        size_t new_cap = self->capacity * 2;
        Ast_Node *new_nodes = realloc(self->nodes, new_cap * sizeof(Ast_Node));
        if (!new_nodes) return 0;
        self->capacity = new_cap;
        self->nodes = new_nodes;
    }

    self->nodes[self->size] = node;
    self->size++;
    return self->size - 1;
}

Ast_Node *Node_Map_Get(Node_Map const *self, size_t i)
{    
    if (!self) return NULL;
    if (i >= self->size) return NULL;
    return &self->nodes[i];
}

void Free_Node_Map(Node_Map *map)
{
    if (!map) return;
    
    if (map->size > 0 && map->nodes[0].type == AST_PROGRAM)
        free_program_node(&map->nodes[0].v_program);

    free(map->nodes);
    free(map);
}

//-------------------------------------------------------------------------------//
// node methods
//-------------------------------------------------------------------------------//

Ast_Node Node_Build(Node_Type type, Span span)
{
    Ast_Node node;
    node.type = type;
    node.span = span;
    return node;
}

void Print_Node(Node_Map *map, size_t id, int i)
{
    if (id == 0) return;
    
    /* print indent */
    for (int j = 0; j < (i); j++) putchar(' ');

    /* get the node */
    Ast_Node *self = Node_Map_Get(map, id);
    if (!self)
    {
        printf("NULL\n");
        return;
    }

    printf("[%zu] : ", id);
    switch (self->type)
    {
        case AST_BINARY_EXPR:
        {
            printf("BINARY EXPR of %s:\n", AST_OP_NAMES[self->v_binary_expr.op]);
            Print_Node(map, self->v_binary_expr.lhs, i + 2);
            Print_Node(map, self->v_binary_expr.rhs, i + 2);
            break;
        }

        case AST_FLOAT:
        {
            printf("FLOAT: %f\n", self->v_float);
            break;
        }

        case AST_INTEGER:
        {
            printf("INTEGER: %ld\n", self->v_integer);
            break;
        }

        case AST_GROUPING:
        {
            printf("GROUPING:\n");
            Print_Node(map, self->v_inner, i + 2);
            break;
        }

        default:
        {
            printf("<Unknown Type>\n");
            break;
        }
    }
}

//-------------------------------------------------------------------------------//
// node builders
//-------------------------------------------------------------------------------//

size_t Node_Build_Integer(Node_Map *map, Span const span, long int value)
{
    Ast_Node node = (Ast_Node) {AST_INTEGER, .v_integer = value, span };
    return Node_Map_Push(map, node);
}

size_t Node_Build_Float(Node_Map *map, Span const span, float value)
{
    Ast_Node node = (Ast_Node) {AST_FLOAT, .v_float = value, span };
    return Node_Map_Push(map, node);
}
