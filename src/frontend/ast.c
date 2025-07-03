#include "frontend/ast.h"
#include "util/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

//===============================================================================//
// NODE FUNCTIONS
//===============================================================================//

size_t Push_And_Get_Id(List *nodes, Ast_Node node)
{
    if (!nodes || !CHECK_LIST_COMPATIBILITY(nodes, Ast_Node))
        return 0;
    
    size_t id = nodes->count;
    List_Add(nodes, &node);
    return id;
}

void Print_Node(List *nodes, size_t id, int i)
{
    if (!nodes || id == 0 || !CHECK_LIST_COMPATIBILITY(nodes, Ast_Node))
        return;

    /* print indent spaces */
    for (int j = 0; j < (i); j++) putchar(' ');

    /* retrieve node */
    Ast_Node *self = List_Get(nodes, id);
    if (!self)
    {
        printf("<NULL>\n");
        return;
    }

    /* print node contents */
    printf("[%zu] : ", id);
    switch (self->type)
    {
        case AST_BINARY_EXPR:
        {
            printf("BINARY EXPR of %s:\n", AST_OP_NAMES[self->v_binary_expr.op]);
            Print_Node(nodes, self->v_binary_expr.lhs, i + 2);
            Print_Node(nodes, self->v_binary_expr.rhs, i + 2);
            break;
        }
        case AST_ASSIGN_EXPR:
        {
            printf("ASSIGNMENT EXPR of %s:\n", AST_OP_NAMES[self->v_assign_expr.op]);
            Print_Node(nodes, self->v_assign_expr.name, i + 2);
            Print_Node(nodes, self->v_assign_expr.value, i + 2);
            break;
        }
        case AST_FLOAT:
        {
            printf("FLOAT: %f\n", self->v_float);
            break;
        }
        case AST_INTEGER:
        {
            printf("INTEGER: %d\n", self->v_integer);
            break;
        }
        case AST_GROUPING:
        {
            printf("GROUPING:\n");
            Print_Node(nodes, self->v_inner, i + 2);
            break;
        }
        case AST_SYMBOL:
        {
            printf("SYMBOL: %s\n", self->v_symbol);
            break;
        }
        default:
        {
            printf("<Unknown Type>\n");
            break;
        }
    }
}

void Node_Free(Ast_Node *self)
{
    if (!self) return;

    switch (self->type)
    {
        case AST_SYMBOL:
        {
            free(self->v_symbol);
            break;
        }

        default: break;
    }
}

//===============================================================================//
// NODE BUILDER FUNCTIONS
//===============================================================================//

Ast_Node Node_Build(Node_Type type, Span span)
{
    return (Ast_Node) {.span = span, .type = type};
}

size_t Node_Build_Integer(List *nodes, Span const span, int32_t value)
{
    Ast_Node node = Node_Build(AST_INTEGER, span);
    node.v_integer = value;
    return Push_And_Get_Id(nodes, node);
}

size_t Node_Build_Float(List *nodes, Span const span, float value)
{
    Ast_Node node = Node_Build(AST_FLOAT, span);
    node.v_float = value;
    return Push_And_Get_Id(nodes, node);
}

size_t Node_Build_Grouping(List *nodes, Span const span, size_t inner)
{
    Ast_Node node = Node_Build(AST_GROUPING, span);
    node.v_inner = inner;
    return Push_And_Get_Id(nodes, node); 
}

size_t Node_Build_Symbol(List *nodes, Span const span, const char *src)
{
    char *raw = Get_Lexeme(src, span.pos, span.len);
    Ast_Node node = Node_Build(AST_SYMBOL, span);
    node.v_symbol = strdup(raw);
    return Push_And_Get_Id(nodes, node); 
}