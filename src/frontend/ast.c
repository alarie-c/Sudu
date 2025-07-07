#include "frontend/ast.h"
#include "util/common.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

//===============================================================================//
// NODE FUNCTIONS
//===============================================================================//

Node_Idx Node_Insert(List *map, Node node)
{
    if (!map || !CHECK_LIST_COMPATIBILITY(map, Node))
        return 0;
    Node_Idx id = map->count;
    List_Add(map, &node);
    return id;
}

#define INDENT(i) for (int j = 0; j < (i); j++) putchar(' ')
void Node_Print(List *map, Node_Idx id, int i)
{
    if (!map || id == 0 || !CHECK_LIST_COMPATIBILITY(map, Node))
        return;
    
    INDENT(i);
    Node *self = List_Get(map, id);
    if (!self)
    {
        printf("<NULL>\n");
        return;
    }

    printf("[%zu] : ", id);
    switch (self->type)
    {
        case NODE_VARIABLE:
        {
            printf("VARIABLE DECL (MUT = %i):\n", self->data.variable.mutability);
            Node_Print(map, self->data.variable.symbol, i + 2);
            if (self->data.variable.initializer != 0)
                Node_Print(map, self->data.variable.initializer, i + 2);
            break;
        }
        case NODE_BINARY:
        {
            printf("BINARY EXPR of %s:\n", OPERATOR_NAMES[self->data.binary.op]);
            Node_Print(map, self->data.binary.lhs, i + 2);
            Node_Print(map, self->data.binary.rhs, i + 2);
            break;
        }
        case NODE_ASSIGNMENT:
        {
            printf("ASSIGNMENT EXPR of %s:\n", OPERATOR_NAMES[self->data.assignment.op]);
            Node_Print(map, self->data.assignment.sym, i + 2);
            Node_Print(map, self->data.assignment.val, i + 2);
            break;
        }
        case NODE_CALL:
        {
            printf("CALL EXPR:\n");
            Node_Print(map, self->data.call.sym, i + 2);
            Node_Print(map, self->data.call.args, i + 2);
            break;
        }
        case NODE_LIST:
        {
            printf("LIST:");
            if (self->data.list.count == 0)
            {
                printf(" <Empty>\n");
                break;
            }
            else
            {
                putchar('\n');
            }
            for (size_t j = 0; j < self->data.list.count; j++)
            {
                Node_Idx id = self->data.list.nodes[j];
                Node_Print(map, id, i + 2);
            }
            break;
        }
        case NODE_FLOAT:
        {
            printf("FLOAT: %f\n", self->data.float_value);
            break;
        }
        case NODE_INTEGER:
        {
            printf("INTEGER: %d\n", self->data.int_value);
            break;
        }
        case NODE_GROUPING:
        {
            printf("GROUPING:\n");
            Node_Print(map, self->data.inner_node, i + 2);
            break;
        }
        case NODE_SYMBOL:
        {
            printf("SYMBOL: %s\n", self->data.symbol_name);
            break;
        }
        default:
        {
            printf("<Unknown Type>\n");
            break;
        }
    }
}

void Node_Free(Node *self)
{
    if (!self) return;
    switch (self->type)
    {
        case NODE_SYMBOL:
        {
            free(self->data.symbol_name);
            break;
        }
        case NODE_LIST:
        {
            free(self->data.list.nodes);
            break;
        }
        default: break;
    }
}

//===============================================================================//
// SUBTYPE FUNCTIONS
//===============================================================================//

void Node_List_Add(Node_List *self, Node_Idx node)
{
    if (!self) return;
    if (self->count >= self->capacity)
    {
        /*(FIX) grow by 50% not 100%*/
        size_t new_capacity = (self->capacity != 0)
                            ? (self->capacity * 2)
                            : 4;
        void *new_nodes = realloc(self->nodes, new_capacity * sizeof(Node_Idx));
        if (!new_nodes) return;

        self->nodes = new_nodes;
        self->capacity = new_capacity;
    }
    self->nodes[self->count] = node;
    self->count++;
}

//===============================================================================//
// NODE BUILDER FUNCTIONS
//===============================================================================//

Node_Idx Make_Node_Integer(List *map, Span const span, int32_t value)
{
    return Node_Insert(map, (Node) {
        .type = NODE_INTEGER,
        .span = span,
        .data.int_value = value,
    });
}

Node_Idx Make_Node_Float(List *map, Span const span, float value)
{
    return Node_Insert(map, (Node) {
        .type = NODE_FLOAT,
        .span = span,
        .data.float_value = value,
    });
}

Node_Idx Make_Node_Grouping(List *map, Span const span, Node_Idx inner)
{
    return Node_Insert(map, (Node) {
        .type = NODE_GROUPING,
        .span = span,
        .data.inner_node = inner,
    });
}

Node_Idx Make_Node_Symbol(List *map, Span const span, const char *src)
{
    char *lexeme = Get_Lexeme(src, span.pos, span.len);
    return Node_Insert(map, (Node) {
        .type = NODE_SYMBOL,
        .span = span,
        .data.symbol_name = lexeme,
    });
}

Node_Idx Make_Node_Binary(List *map, Span const span, Node_Idx lhs, Node_Idx rhs, Operator op)
{
    return Node_Insert(map, (Node) {
        .type = NODE_BINARY,
        .span = span,
        .data.binary = (Node_Binary) {
            .lhs = lhs,
            .rhs = rhs,
            .op = op,
        },
    });
}

Node_Idx Make_Node_Assignment(List *map, Span const span, Node_Idx sym, Node_Idx val, Operator op)
{
        return Node_Insert(map, (Node) {
        .type = NODE_ASSIGNMENT,
        .span = span,
        .data.assignment = (Node_Assignment) {
            .sym = sym,
            .val = val,
            .op = op,
        },
    });
}

Node_Idx Make_Node_Variable(List *map, Span const span, Node_Idx sym, Node_Idx initializer, bool mut)
{
    return Node_Insert(map, (Node) {
        .type = NODE_VARIABLE,
        .span = span,
        .data.variable = (Node_Variable) {
            .symbol = sym,
            .initializer = initializer,
            .mutability = mut,
        },
    });
}

Node_Idx Make_Node_Call(List *map, Span const span, Node_Idx sym, Node_Idx args)
{
    return Node_Insert(map, (Node) {
        .type = NODE_CALL,
        .span = span,
        .data.call = (Node_Call) {
            .sym = sym,
            .args = args,
        },
    });
}

Node_Idx Make_Node_List(List *map, Span const start_span, size_t capacity)
{
    Node_List self = {0};
    List basic_list = List_New(sizeof(Node_Idx), capacity);
    
    self.nodes = basic_list.data;
    self.capacity = capacity;
    return Node_Insert(map, (Node) {
        .type = NODE_LIST,
        .span = start_span,
        .data.list = self,
    });
}

//===============================================================================//
// ABSTRACT SYNTAX TREE
//===============================================================================//

Node_Idx make_node_root(List *map, size_t capacity)
{
    Node_List self = {0};
    List basic_list = List_New(sizeof(Node_Idx), capacity);
    
    self.nodes = basic_list.data;
    self.capacity = capacity;
    return Node_Insert(map, (Node) {
        .type = NODE_ROOT,
        .span = (Span) {0},
        .data.root = self,
    });
}

AST AST_Init()
{
    List node_map = List_New(sizeof(Node), INIT_NODE_MAP_CAPACITY);
    
    Node_Idx root_idx = make_node_root(&node_map, INIT_ROOT_NODE_CAPACITY);
    Node *root_node = List_Get(&node_map, root_idx);

    return (AST) {
        .node_map = node_map,
        .root_node = root_node,
    };
}

void AST_Insert(AST *self, Node_Idx node_idx)
{
    Node_List_Add(&self->root_node->data.root, node_idx);
}