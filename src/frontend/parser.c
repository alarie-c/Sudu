#include "frontend/parser.h"
#include "frontend/lexer.h";
#include "frontend/ast.h"
#include "util/errors.h"
#include "util/tests.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define CHECK_NONZERO(node) if ((node) == 0) return 0

typedef struct parser parser;

//-------------------------------------------------------------------------------//
// error methods
//-------------------------------------------------------------------------------//

static void error_on_token(List *errors, Token tk, const char *msg)
{
    if (!errors) return;
    CHECK_LIST_COMPAT_TYPE(errors, Error);
    Error err = Make_Error(ERR_SYNTAX, tk.x, tk.y, tk.span, msg);
    List_Add(errors, &err);
}

//-------------------------------------------------------------------------------//
// mini parsers
//-------------------------------------------------------------------------------//

static bool into_integer(char *lexeme, int32_t *value)
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

static bool into_float(char *lexeme, float *value)
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
// parser state
//-------------------------------------------------------------------------------//

struct parser
{
    const char *src;
    Token *token_stream;
    size_t token_count;
    size_t pos;
    Ast_Node *root;
    List *errors;
    List *map;
};

static Token next(parser *self)
{
    if (self->pos + 1 >= self->token_count)
        return self->token_stream[self->token_count - 1];   
    return self->token_stream[self->pos++];
}

static Token peek(parser *self)
{
    if (self->pos + 1 >= self->token_count)
        return self->token_stream[self->token_count - 1];
    return self->token_stream[self->pos + 1];
}

static Token current(parser *self)
{
    if (self->pos >= self->token_count)
        return self->token_stream[self->token_count - 1];
    return self->token_stream[self->pos];
}

//-------------------------------------------------------------------------------//
// parser methods
//-------------------------------------------------------------------------------//

/// FWD DECLARATION
static size_t parse_expr(parser *self);

static size_t parse_literal(parser *self)
{
    Token tk = current(self);
    switch (tk.kind)
    {
        case TOK_INTEGER:
        {
            size_t size = tk.span.len + 1;
            size_t buffer_size = size >= 3 ? size : 3;
            char lex[buffer_size];
            int32_t value;
            
            Get_Lexeme(lex, buffer_size, self->src, &tk.span, NO_ESCAPES);
            int result = into_integer(lex, &value);
            if (!result) return 0;

            size_t pos = tk.span.pos;
            size_t len = tk.span.len;
            Span node_span = (Span) {pos, len};

            return Node_Build_Integer(self->map, node_span, value);
        }

        case TOK_FLOAT:
        {
            size_t size = tk.span.len + 1;
            size_t buffer_size = size >= 3 ? size : 3;
            char lex[buffer_size];
            float value;
            
            Get_Lexeme(lex, buffer_size, self->src, &tk.span, NO_ESCAPES);
            int result = into_float(lex, &value);
            if (!result) return 0;

            size_t pos = tk.span.pos;
            size_t len = tk.span.len;
            Span node_span = (Span) {pos, len};

            return Node_Build_Float(self->map, node_span, value);
        }

        case TOK_LPAREN:
        {
            next(self); /* consume LPAREN */
            
            size_t inner = parse_expr(self);
            CHECK_NONZERO(inner);

            Token peeked = peek(self);
            if (peeked.kind != TOK_RPAREN)
            {
                error_on_token(self->errors, peeked, "expected ')'");
                return 0;     
            }
            next(self); /* consume RPAREN */

            size_t len = current(self).span.len;
            Span span = (Span) {tk.span.pos, 1 + len};

            Ast_Node node = Node_Build(AST_GROUPING, span);
            node.v_inner = inner;

            return Push_And_Get_Id(self->map, node);           
        }
        
        default:
        {
            return 0;
        }
    }
}

static size_t parse_binary(parser *self)
{
    Token tk = current(self);
    size_t expr = parse_literal(self);
    CHECK_NONZERO(expr);
    
    Ast_Op op = Get_Binary_Infix_Operator(peek(self).kind);
    if (op != -1)
    {
        next(self); /* operator */
        next(self); /* beginning of RHS */
        Span span = (Span) {tk.span.pos, tk.span.len + current(self).span.len};
       
        size_t rhs = parse_expr(self);
        CHECK_NONZERO(rhs);

        Ast_Binary_Expr value = (Ast_Binary_Expr) {.lhs = expr, .op = op, .rhs = rhs};
        Ast_Node node = Node_Build(AST_BINARY_EXPR, span);
        
        node.v_binary_expr = value;
        expr = Push_And_Get_Id(self->map, node);
    }

    return expr;
}

static size_t parse_expr(parser *self)
{
    return parse_binary(self);
}

/* Tests */
static bool test_parser(Test_Info *info, const char *src)
{
    /* tokenize everything */
    List tokens = Tokenize(src);
    List errors = List_New(sizeof(Error), INIT_ERROR_CAPACITY);

    /* create the root node */
    List subnodes = List_New(sizeof(Ast_Node), INIT_PROGRAM_CAPACITY);
    Ast_Node root = (Ast_Node) {
        .span = (Span) {0},
        .type = AST_PROGRAM, 
        .v_root = subnodes,
    };

    /* create the node map */
    List map = List_New(sizeof(Ast_Node), INIT_PROGRAM_CAPACITY * 2);
    Ast_Node null_node = (Ast_Node) {0};
    List_Add(&map, &null_node); /* insert dummy null node */
    
    /* create the state */
    parser self = (parser) {
        .src = src,
        .token_stream = (Token *)tokens.data,
        .token_count = tokens.count,
        .pos = 0,
        .errors = &errors,
        .root = &root,
        .map = &map,
    };

    do {
        /* parse one expr */
        size_t expr = parse_expr(&self);
        
        Report_Errors(&errors, src, "<TestPath>");
        if (expr == 0)
        {
            info->message = "zero ID";
            info->status = false;
            info->success = false;
            return false;
        }
        
        /* add this expr to the program node */
        Print_Node(&map, expr, 0);
        List_Add(&root.v_root, &expr);
        next(&self);

        if (current( &self).kind == TOK_EOF) break;
    } while(1);
    
    info->status = true;
    info->success = true;

    return true;
}

void Test_Parser(Test_Info *info)
{
    const char *src = "(5 + 10";
    if (!test_parser(info, src)) return;

    info->success = true;
    info->status = true;
}