#include "common.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

//-------------------------------------------------------------------------------//
// node map methods
//-------------------------------------------------------------------------------//

Ast_Node init_program_node(size_t cap)
{
    size_t *ids = (size_t *)malloc(cap * sizeof(size_t));
    if (!ids) return (Ast_Node) {1};

    Ast_Program program = (Ast_Program) {ids, 0, cap};
    return (Ast_Node) {AST_PROGRAM, program, (Span) {0}};
}

void free_program_node(Ast_Program *self)
{
    if (!self) return;
    free(self->ids);
}

Node_Map *node_map_init(size_t cap)
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

    Ast_Node program = init_program_node(INIT_PROGRAM_CAPACITY);
    if (program.type != AST_PROGRAM)
    {
        free(nodes);
        return NULL;
    }

    ast->nodes[0] = program;
    return ast;
}

size_t node_map_push(Node_Map *self, Ast_Node node)
{
    if (!self) return 0;

    if (self->size == self->capacity)
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

Ast_Node *node_map_get(Node_Map const *self, size_t i)
{
    if (!self) return NULL;
    if (i >= self->size) return NULL;
    return &self->nodes[i];
}

void Free_Node_Map(Node_Map *map)
{
    if (!map) return;
    
    if (map->size > 0 && map->nodes[0].type == AST_PROGRAM)
        free_program_node(&map->nodes[0].data_program);

    free(map->nodes);
    free(map);
}

//-------------------------------------------------------------------------------//
// program node methods
//-------------------------------------------------------------------------------//

void push_to_program(Parser *self, Ast_Program *root, Ast_Node node)
{
    if (!root || !self) return;

    size_t id = node_map_push(self->map, node);
    if (id == 0) return;

    if (root->size == root->capacity)
    {
        size_t new_cap = root->capacity * 2;
        size_t *new_ids = (size_t *)realloc(root->ids, new_cap * sizeof(size_t));
        if (!new_ids) return;
        root->ids = new_ids;
        root->capacity = new_cap;
    }

    root->ids[root->size] = id;
}

//-------------------------------------------------------------------------------//
// node methods
//-------------------------------------------------------------------------------//

void print_indent(int i)
{
    for (int j = 0; j < i; j++)
        putchar(' ');
}

void Print_Node(Node_Map *map, size_t id, int i)
{
    if (id == 0) return;
    
    print_indent(i);
    Ast_Node *self = node_map_get(map, id);
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
            printf("BINARY EXPR:\n");
            Print_Node(map, self->data_binary_expr.lhs, i + 2);
            Print_Node(map, self->data_binary_expr.rhs, i + 2);
            break;
        }

        case AST_FLOAT:
        {
            printf("FLOAT: %f\n", self->data_float);
            break;
        }

        case AST_INTEGER:
        {
            printf("INTEGER: %ld\n", self->data_integer);
        }
    }
}

size_t make_node_integer(Node_Map *map, Span const span, long int value)
{
    Ast_Node node = (Ast_Node) {AST_INTEGER, .data_integer = value, span };
    return node_map_push(map, node);
}

size_t make_node_float(Node_Map *map, Span const span, float value)
{
    Ast_Node node = (Ast_Node) {AST_FLOAT, .data_float = value, span };
    return node_map_push(map, node);
}

//-------------------------------------------------------------------------------//
// parser methods
//-------------------------------------------------------------------------------//

Parser *Init_Parser(const char *src)
{
    Parser *parser = malloc(sizeof(Parser));
    Lexer *lexer = Init_Lexer(src);
    if (lexer == NULL || parser == NULL) return NULL;

    // create the AST program node
    Node_Map *map = node_map_init(INIT_AST_CAPACITY);
    if (map == NULL) return NULL;

    parser->src = src;
    parser->lexer = lexer;
    parser->map = map;
    return parser;
}

void Free_Parser(Parser *self)
{
    Free_Lexer(self->lexer);
    Free_Node_Map(self->map);
    free(self);
}

void remove_literal_underscores(char *str)
{
    char *src = str;
    char *dst = str;
    while (*src)
    {
        if (*src != '_')
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

//-------------------------------------------------------------------------------//
// parser helpers
//-------------------------------------------------------------------------------//

bool parse_integer(char *lexeme, long int *value)
{
    remove_literal_underscores(lexeme);

    char *endptr;
    long int num;
    num = strtol(lexeme, &endptr, 10);
    if (endptr == lexeme)
    {
        printf("No digits in INTEGER\n");
        return false;
    }
    else if (*endptr != '\0')
    {
        printf("Invalid chars in INTEGER\n");
        return false;
    }

    *value = num;
    return true;
}

int parse_float(char *lexeme, float *value)
{
    remove_literal_underscores(lexeme);

    char *endptr;
    float num;
    num = strtof(lexeme, &endptr);
    if (endptr == lexeme)
    {
        printf("No digits in FLOAT\n");
        return false;
    }
    else if (*endptr != '\0')
    {
        printf("Invalid chars in FLOAT\n");
        return false;
    }

    *value = num;
    return true;
}

//-------------------------------------------------------------------------------//
// actual parsers
//-------------------------------------------------------------------------------//

size_t parse_literal(Parser *self, Token *tk)
{   
    Node_Map *map = self->map;
    
    switch (tk->kind)
    {
        case TOK_INTEGER:
        {
            size_t size = tk->span.len + 1;
            size_t buffer_size = size >= 3 ? size : 3;
            char lex[buffer_size];
            long int value;
            
            Get_Lexeme(lex, buffer_size, self->src, &tk->span, NO_ESCAPES);
            int result = parse_integer(lex, &value);
            if (!result) return 0;

            size_t pos = tk->span.pos;
            size_t len = tk->span.len;
            Span node_span = (Span) {pos, len};

            return make_node_integer(map, node_span, value);
        }

        case TOK_FLOAT:
        {
            size_t size = tk->span.len + 1;
            size_t buffer_size = size >= 3 ? size : 3;
            char lex[buffer_size];
            float value;
            
            Get_Lexeme(lex, buffer_size, self->src, &tk->span, NO_ESCAPES);
            int result = parse_float(lex, &value);
            if (!result) return 0;

            size_t pos = tk->span.pos;
            size_t len = tk->span.len;
            Span node_span = (Span) {pos, len};

            return make_node_float(map, node_span, value);
        }
        
        default:
        {
            return 0;
        }
    }
}