#include "parser.h"
#include "ast.h"
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

//-------------------------------------------------------------------------------//
// error methods
//-------------------------------------------------------------------------------//

void throw_tokens(Parser *self, Token tk, const char *msg)
{
    if (!self) return;
    Ec_Push(self->errors, Make_Error(ERR_SYNTAX, tk.x, tk.y, tk.span, msg));
}

//-------------------------------------------------------------------------------//
// parser methods
//-------------------------------------------------------------------------------//

Parser *Init_Parser(const char *src)
{
    Parser *parser = malloc(sizeof(Parser));
    Lexer *lexer = Init_Lexer(src);
    if (!lexer || !parser) return NULL;

    Node_Map *map = Node_Map_Init(INIT_AST_CAPACITY);
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
    
    Token t = (Token) {0};
    parser->peeked_tk = t;

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
    Remove_Underbars(lexeme);

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
    Remove_Underbars(lexeme);

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

void push_to_program(Parser *self, Ast_Program *root, Ast_Node node)
{
    if (!root || !self) return;

    size_t id = Node_Map_Push(self->map, node);
    if (id == 0) return;

    if (root->size == root->capacity)
    {
        size_t new_cap = root->capacity * 2;
        size_t *new_ids = realloc(root->ids, new_cap * sizeof(size_t));
        if (!new_ids) return;
        root->ids = new_ids;
        root->capacity = new_cap;
    }

    root->ids[root->size] = id;
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

            return Node_Build_Integer(self->map, node_span, value);
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

            return Node_Build_Float(self->map, node_span, value);
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

            Ast_Node node = Node_Build(AST_GROUPING, span);
            node.v_inner = inner;

            return Node_Map_Push(self->map, node);
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
        Ast_Node node = Node_Build(AST_BINARY_EXPR, span);
        
        node.v_binary_expr = value;
        expr = Node_Map_Push(self->map, node);
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
        return false;
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

    return true;
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