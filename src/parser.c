#include "parser.h"
#include "tests.h"
#include "common.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#define CHECK_NONZERO(node) if ((node) == 0) return 0
#define PRINT_NODE(id) Print_Node(self->map, id, 2)

void throw_tokens(Parser *self, Token tk, const char *msg)
{
    if (!self) return;
    Ec_Push(self->errors, Make_Error(ERR_SYNTAX, tk.x, tk.y, tk.span, msg));
}

//-------------------------------------------------------------------------------//
// node map methods
//-------------------------------------------------------------------------------//

Ast_Node make_node(Node_Type type, Span span)
{
    Ast_Node node;
    node.type = type;
    node.span = span;
    return node;
}

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

    if (self->size >= self->capacity)
    {
        size_t new_cap = self->capacity * 2;
        Ast_Node *new_nodes = (Ast_Node *)realloc(self->nodes, new_cap * sizeof(Ast_Node));
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
        free_program_node(&map->nodes[0].v_program);

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
    for (int j = 0; j < i; j++) putchar(' ');
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

size_t make_node_integer(Node_Map *map, Span const span, long int value)
{
    Ast_Node node = (Ast_Node) {AST_INTEGER, .v_integer = value, span };
    return node_map_push(map, node);
}

size_t make_node_float(Node_Map *map, Span const span, float value)
{
    Ast_Node node = (Ast_Node) {AST_FLOAT, .v_float = value, span };
    return node_map_push(map, node);
}

//-------------------------------------------------------------------------------//
// parser methods
//-------------------------------------------------------------------------------//

Parser *Init_Parser(const char *src)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    Lexer *lexer = Init_Lexer(src);
    if (!lexer || !parser) return NULL;

    Node_Map *map = node_map_init(INIT_AST_CAPACITY);
    if (!map)
    {
        Free_Lexer(lexer);
        free(parser);
        return NULL;
    }

    Error_Collection *ec = Init_Error_Collection(parser->src, "<NoPath>");
    if (!ec)
    {
        Free_Lexer(lexer);
        Free_Node_Map(map);
        free(parser);
        return NULL;
    }
    
    Token t; parser->peeked_tk = t;

    parser->src = src;
    parser->lexer = lexer;
    parser->errors = ec;
    parser->map = map;
    
    parser->is_peeked = false;
    parser->is_at_end = false;

    parser->current_tk = Next_Token(parser->lexer);
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

Token p_next(Parser *self)
{
    if (self->is_peeked)
    {
        Token tk = self->peeked_tk;
        self->peeked_tk = (Token) {0};
        self->is_peeked = false;
        self->current_tk = tk;
        return tk;
    }
    Token tk = Next_Token(self->lexer);
    self->current_tk = tk;

    if (tk.kind == TOK_EOF) self->is_at_end = true;
    return tk;
}

Token p_peek(Parser *self)
{
    if (self->is_peeked)
    {
        return self->peeked_tk;
    }

    Token tk = Next_Token(self->lexer);
    self->peeked_tk = tk;
    self->is_peeked = true;
    return tk;
}

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

size_t parse_expr(Parser *self);

size_t parse_literal(Parser *self)
{   
    Token *tk = &self->current_tk;
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

            return make_node_integer(self->map, node_span, value);
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

            return make_node_float(self->map, node_span, value);
        }

        case TOK_LPAREN:
        {
            p_next(self); /* consume LPAREN */
            
            size_t inner = parse_expr(self);
            CHECK_NONZERO(inner);

            if (p_peek(self).kind != TOK_RPAREN)
            {
                throw_tokens(self, self->peeked_tk, "expected ')'");
                return 0;
            }
            p_next(self); /* consume RPAREN */

            size_t len = self->current_tk.span.len;
            Span span = (Span) {tk->span.pos, 1 + len};

            Ast_Node node = make_node(AST_GROUPING, span);
            node.v_inner = inner;

            return node_map_push(self->map, node);
        }
        
        default:
        {
            return 0;
        }
    }
}

size_t parse_binary(Parser *self)
{
    Token tk = self->current_tk;
    size_t expr = parse_literal(self);
    CHECK_NONZERO(expr);

    Ast_Op op = AST_BINARY_INFIX[p_peek(self).kind];
    if (op != OP_NONE)
    {
        p_next(self); /* operator */
        p_next(self); /* beginning of RHS */
        Span span = (Span) {tk.span.pos, tk.span.len + self->current_tk.span.len};
       
        size_t rhs = parse_expr(self);
        CHECK_NONZERO(rhs);

        Ast_Binary_Expr value = (Ast_Binary_Expr) {.lhs = expr, .op = op, .rhs = rhs};
        Ast_Node node = make_node(AST_BINARY_EXPR, span);
        
        node.v_binary_expr = value;
        expr = node_map_push(self->map, node);
    }

    return expr;
}

size_t parse_expr(Parser *self)
{
    return parse_binary(self);
}

//-------------------------------------------------------------------------------//
// parser tests
//-------------------------------------------------------------------------------//

bool test_parser(Test_Info *info, const char *src)
{
    printf("> Source: '%s'\n", src);
    
    Parser *parser = Init_Parser(src);
    if (!parser)
    {
        info->success = false;
        info->status = false;
        info->message = "parser failed to allocate";
    }

    bool eof = false;
    do {
        size_t id = parse_expr(parser);
        
        if (id == 0)
        {
            printf("<NULL NODE>\n");
        }
        else
        {
            printf("> Printing Node:\n");
            Print_Node(parser->map, id, 4);
        }
        
        p_next(parser);
        if (parser->is_at_end) eof = true;
    } while (!eof);

    Ec_Report_All(parser->errors);
    Ec_Free(parser->errors);
    Free_Parser(parser);
}

void Test_Grouping_Expression(Test_Info *info)
{
    const char *src = "(5 + 10) + (50 + 100)";
    if (!test_parser(info, src)) return;

    info->success = true;
    info->status = true;
}

void Test_Binary_Expression(Test_Info *info)
{
    const char *src = "5 + 10 * 15 / 800";
    if (!test_parser(info, src)) return;

    info->success = true;
    info->status = true;
}