#include "parser.h"
#include "ast.h"
#include "common.h"
#include "tests.h"
#include "lexer.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

void error_on_token(List *errors, Token tk, const char *msg)
{
    if (!errors) return;
    Error err = Make_Error(ERR_SYNTAX, tk.x, tk.y, tk.span, msg);
    List_Add(errors, &err);
}

//-------------------------------------------------------------------------------//
// parser methods
//-------------------------------------------------------------------------------//

Token parser_next(Parser *self)
{
    if (!self) return (Token) {0};
    
    if (self->is_peeked)
    {
        
        /* get the peeked token and reset */
        Token tk = self->peeked_tk;
        self->peeked_tk = (Token) {0};
        self->is_peeked = false;
        
        /* update EOF condition */
        if (tk.kind == TOK_EOF) self->is_at_end = true;
        return tk;
    }

    Token tk = Next_Token(self->lexer);
    self->current_tk = tk;
    if (tk.kind == TOK_EOF) self->is_at_end = true;
    return tk;
}

Token parser_peek(Parser *self)
{
    if (!self) return (Token) {0};
    if (self->is_peeked) return self->peeked_tk;
    
    Token tk = Next_Token(self->lexer);
    self->is_peeked = true;
    self->peeked_tk = tk;
    return tk;
}

Parser *Parser_Init(const char *src, const char *path)
{
    Parser *self = malloc(sizeof(Parser));
    if (!self) return NULL;

    Lexer *lexer = Init_Lexer(src);
    if (!lexer)
    {
        free(self);
        return NULL;
    }

    self->lexer = lexer;
    self->src = src;
    self->path = strdup(path);
    self->is_at_end = false;
    self->is_peeked = false;
    
    self->current_tk = Next_Token(lexer);
    self->peeked_tk = (Token) {0};

    List syntax_tree = List_New(sizeof(size_t), INIT_PROGRAM_CAPACITY);
    List nodes = List_New(sizeof(Ast_Node), INIT_PROGRAM_CAPACITY);
    List errors = List_New(sizeof(Error), INIT_ERROR_CAPACITY);
    if (nodes.capacity == 0
        || errors.capacity == 0
        || syntax_tree.capacity == 0)
    {
        Free_Lexer(self->lexer);
        free(self);
        return NULL;
    }  

    Ast_Node base = Node_Build(AST_PROGRAM, (Span) {0});
    base.v_program = syntax_tree;
    List_Add(&nodes, &base);

    self->nodes = nodes;
    self->errors = errors;
    return self;
}

void Parser_Free(Parser *self)
{
    if (!self) return;
    Free_Lexer(self->lexer);
    List_Free(&self->nodes);
    List_Free(&self->errors);
    free(self->path);
    free(self);
}

void Parse(Parser *self)
{}

//-------------------------------------------------------------------------------//
// helper parsers
//-------------------------------------------------------------------------------//

bool parse_integer(char *lexeme, int32_t *value)
{
    Remove_Underbars(lexeme);

    char *endptr;
    int32_t num;
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

//-------------------------------------------------------------------------------//
// parser implementation
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
            int32_t value;
            
            Get_Lexeme(lex, buffer_size, self->src, &tk->span, NO_ESCAPES);
            int result = parse_integer(lex, &value);
            if (!result) return 0;

            size_t pos = tk->span.pos;
            size_t len = tk->span.len;
            Span node_span = (Span) {pos, len};

            return Node_Build_Integer(&self->nodes, node_span, value);
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

            return Node_Build_Float(&self->nodes, node_span, value);
        }
        case TOK_LPAREN:
        {
            parser_next(self); /* consume LPAREN */
            
            size_t inner = parse_expr(self);
            CHECK_NONZERO(inner);

            if (parser_peek(self).kind != TOK_RPAREN)
            {
                error_on_token(&self->errors, self->peeked_tk, "expected ')'");
                return 0;
            }
            parser_next(self); /* consume RPAREN */

            size_t len = self->current_tk.span.len;
            Span node_span = (Span) {tk->span.pos, 1 + len};

            return Node_Build_Grouping(&self->nodes, node_span, inner);
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

    Ast_Op op = AST_BINARY_INFIX[parser_peek(self).kind];
    if (op != OP_NONE)
    {
        parser_next(self); /* operator */
        parser_next(self); /* beginning of RHS */
        Span span = (Span) {tk.span.pos, tk.span.len + self->current_tk.span.len};
       
        size_t rhs = parse_expr(self);
        CHECK_NONZERO(rhs);

        Ast_Binary_Expr value = (Ast_Binary_Expr) {.lhs = expr, .op = op, .rhs = rhs};
        Ast_Node node = Node_Build(AST_BINARY_EXPR, span);
        node.v_binary_expr = value;

        expr = Push_And_Get_Id(&self->nodes, node);
    }

    return expr;
}

size_t parse_expr(Parser *self)
{
    PRINT_TOKEN(self->current_tk);
    return parse_binary(self);
}

//-------------------------------------------------------------------------------//
// parser tests
//-------------------------------------------------------------------------------//

bool test_parser(Test_Info *info, const char *src)
{
    printf("> Source: '%s'\n", src);
    
    Parser *parser = Parser_Init(src, "<No Path>");
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
            Print_Node(&parser->nodes, id, 4);
        }
        
        parser_next(parser);
        if (parser->is_at_end) eof = true;
    } while (!eof);

    Report_Errors(&parser->errors, parser->src, parser->path);
    Parser_Free(parser);

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