#include "frontend/frontend.h"
#include "frontend/ast.h"
#include "util/tests.h"
#include "util/common.h"
#include "util/errors.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//===============================================================================//
// LEXER MACROS
//===============================================================================//

#define LEX_IS_EOF ((self->pos) >= (self->len))
#define LEX_SPAN(len) ((Span) {self->pos, (len)})
#define LEX_SPAN_DOUBLE ((Span) {self->pos - 1, 2})

//===============================================================================//
// LEXER IMPLEMENTATION
//===============================================================================//

typedef struct _lexer
{
    const char *src;
    size_t len;
    size_t pos;
    size_t x;
    size_t y;
} lexer;

static Token_Kind lex_cmp_keywords(const char *src, const Span *span)
{
    if (Cmp_Lexeme(src, span, "proc"))
        return TOK_PROC;
    else if (Cmp_Lexeme(src, span, "let"))
        return TOK_LET;
    else if (Cmp_Lexeme(src, span, "mut"))
        return TOK_MUT;
    else
        return TOK_SYMBOL;
}

static void lex_consume(lexer *self)
{
    if (LEX_IS_EOF)
        return;
    self->pos += 1;
    self->x += 1;
}

static char lex_peek(const lexer *self)
{
    if (self->pos + 1 >= self->len)
        return '\0';
    return self->src[self->pos + 1];
}

static char lex_current_char(const lexer *self)
{
    if (LEX_IS_EOF)
        return '\0';
    return self->src[self->pos];
}

static void lex_eat_whitespace(lexer *self)
{
    char ch = lex_current_char(self);
    while (ch == ' ' || ch == '\t' || ch == '\r')
    {
        lex_consume(self);
        ch = lex_current_char(self);
    }
}

//===============================================================================//
// TOKENIZER
//===============================================================================//

static Token lex_next_token(lexer *self)
{

    if (LEX_IS_EOF)
        return (Token) {TOK_EOF, LEX_SPAN(1), self->x, self->y};
    
    lex_eat_whitespace(self);
    char ch = lex_current_char(self);
    size_t start = self->x;
    Token tk;

    switch (ch)
    {
        case '\n':
        {
            tk = (Token) {TOK_NEWLINE, LEX_SPAN(1), self->x, self->y};
            self->y++;
            self->x = 0; /* zero here because consume() will advance it to 1 */
            break;
        }
        case '(':
        {
            tk = (Token) {TOK_LPAREN, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case ')':
        {
            tk = (Token) {TOK_RPAREN, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '=':
        {
            if (lex_peek(self) == '=')
            {
                lex_consume(self);
                tk = (Token) {TOK_EQ_EQ, LEX_SPAN_DOUBLE, start, self->y};
                break;
            }
            tk = (Token) {TOK_EQ, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '+':
        {
            if (lex_peek(self) == '=')
            {
                lex_consume(self);
                tk = (Token) {TOK_PLUS_EQ, LEX_SPAN_DOUBLE, start, self->y};
                break;
            }
            tk = (Token) {TOK_PLUS, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '-':
        {
            tk = (Token) {TOK_MINUS, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '*':
        {
            tk = (Token) {TOK_STAR, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '/':
        {
            tk = (Token) {TOK_SLASH, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case '%':
        {
            tk = (Token) {TOK_PERCENT, LEX_SPAN(1), self->x, self->y};
            break;
        }
        case ',':
        {
            tk = (Token) {TOK_COMMA, LEX_SPAN(1), self->x, self->y};
            break;
        }
        
        // handle comments
        case '#':
        {
            do {
                if (lex_current_char(self) == '\n'
                    || lex_current_char(self) == '\0')
                {
                    break;
                }
                lex_consume(self);
            } while (true);

            return lex_next_token(self);
        }

        default:
        {
            if (isalpha(ch) || ch == '_')
            {
                size_t start = self->pos;
                size_t startx = self->x;

                do {
                    ch = lex_peek(self);
                    if (!isalnum(ch) && ch != '_')
                        break;
                    lex_consume(self);
                } while (true);

                size_t end = self->pos;
                Span span = (Span) {start, end - start + 1};
                Token_Kind kind = lex_cmp_keywords(self->src, &span);
                
                tk = (Token) {kind, span, startx, self->y};
                break; 
            }
            
            if (isdigit(ch))
            {
                Token_Kind kind = TOK_INTEGER;
                size_t start = self->pos;
                size_t startx = self->x;

                do {
                    ch = lex_peek(self);
                    if (isdigit(ch) || ch == '_' || ch == '.')
                    {
                        if (ch == '.' && kind == TOK_FLOAT)
                        {
                            // float already lexed, this `.` is for something else
                            break;
                        }
                        else if (ch == '.' && kind == TOK_INTEGER)
                        {
                            kind = TOK_FLOAT;
                        }
                        lex_consume(self);
                    }
                    else
                    {
                        break;
                    }
                } while (true);

                size_t end = self->pos;
                Span span = (Span) {start, end - start + 1};
                tk = (Token) {kind, span, startx, self->y};
                break; 
            }
            
            tk = (Token) {TOK_ILLEGAL, LEX_SPAN(1), self->x, self->y};
            break;
        }
    }

    lex_consume(self);
    return tk;
}

static List tokenize(const char *src)
{
    List tokens = List_New(sizeof(Token), INIT_TOKEN_CAPACITY);
    if (tokens.capacity == 0) return (List) {0};

    /* create the state struct */
    lexer lex = (lexer) { 
        .src = src,
        .len = strlen(src),
        .pos = 0,
        .x = 1, .y = 1
    };

    do {
        Token next = lex_next_token(&lex);
        List_Add(&tokens, &next);
        
        if (next.kind == TOK_EOF) break;
    } while(1);

    return tokens;
}

//===============================================================================//
// SHORTHAND PARSER ERROR METHODS
//===============================================================================//

#define SPAN_FROM_THEN_TO_CURRENT(then, current) (Span) {(then).span.pos, (current).span.pos - (then).span.pos}
#define PARSER_CHECK_INVALID_NODE(node) if ((node) == 0) return 0
#define PARSER_EMIT_ERR(error_type, x, y, span, msg)                           \
    Error err = Make_Error((error_type), (x), (y), (span), (msg));             \
    List_Add(self->errors, &err)                                               \

static void error_on_token(List *errors, Token tk, const char *msg)
{
    if (!errors) return;
    CHECK_LIST_COMPATIBILITY(errors, Error);
    Error err = Make_Error(ERR_SYNTAX, tk.x, tk.y, tk.span, msg);
    List_Add(errors, &err);
}

//===============================================================================//
// PARSER HELPER FUNCTIONS
//===============================================================================//

static bool into_integer(char *lexeme, int32_t *value)
{
    Remove_Underbars(lexeme);

    char *endptr; int32_t num;
    num = strtol(lexeme, &endptr, 10);
    
    if (endptr == lexeme || *endptr != '\0')
        return false;

    *value = num;
    return true;
}

static bool into_float(char *lexeme, float *value)
{
    Remove_Underbars(lexeme);

    char *endptr; float num;
    num = strtof(lexeme, &endptr);
    
    if (endptr == lexeme || *endptr != '\0')
        return false;

    *value = num;
    return true;
}

//===============================================================================//
// PARSER IMPLEMENTATION
//===============================================================================//

typedef struct _parser
{
    const char *src;
    Ast_Node *root;
    Token *token_stream;
    size_t token_count;
    size_t pos;
    List *errors;
    List *map;
} parser;

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

//===============================================================================//
// PARSER FUNCTIONS
//===============================================================================//

/// FWD DECLARATION
static Node_Idx parse_expr(parser *self);

static Node_Idx parse_literal(parser *self)
{
    Token tk = current(self);
    switch (tk.kind)
    {
        case TOK_INTEGER:
        {
            int32_t value;
            char *raw = Get_Lexeme(self->src, tk.span.pos, tk.span.len);

            if (!into_integer(raw, &value))
            {
                PARSER_EMIT_ERR(ERR_INVALID_LITERAL, tk.x, tk.y, tk.span, "invalid integer literal");
                return 0;
            }

            return Node_Build_Integer(self->map, tk.span, value);
        }

        case TOK_FLOAT:
        {
            float value;
            char *raw = Get_Lexeme(self->src, tk.span.pos, tk.span.len);

            if (!into_float(raw, &value))
            {
                PARSER_EMIT_ERR(ERR_INVALID_LITERAL, tk.x, tk.y, tk.span, "invalid float literal");
                return 0;
            }

            return Node_Build_Float(self->map, tk.span, value);
        }

        case TOK_SYMBOL:
        {
            return Node_Build_Symbol(self->map, tk.span, self->src);
        }

        case TOK_LPAREN:
        {
            next(self); /* consume LPAREN */
            
            Node_Idx inner = parse_expr(self);
            PARSER_CHECK_INVALID_NODE(inner);

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

static Node_Idx parse_call(parser *self)
{
    Token tk = current(self);
    Node_Idx expr = parse_literal(self);
    PARSER_CHECK_INVALID_NODE(expr);

    if (peek(self).kind == TOK_LPAREN)
    {
        next(self); /* LPAREN */
        next(self); /* beginning of args */

        /* initialize a list of arguments */
        Ast_List args = Ast_List_New(INIT_PROC_ARGS_ARITY);
        if (args.capacity == 0)
            return 0;
        size_t args_start = current(self).span.pos;

        if (current(self).kind != TOK_RPAREN)
        {
            do {
                Node_Idx arg = parse_expr(self);
                PARSER_CHECK_INVALID_NODE(arg);

                Ast_List_Push(&args, arg);
                next(self);
                
                /* look for COMMA and continue */
                Token current_token = current(self);
                if (current_token.kind == TOK_COMMA)
                {
                    next(self); /* COMMA */
                    continue;
                }

                /* look for the closing RPAREN */
                else if (current_token.kind == TOK_RPAREN)
                {
                    break;
                }

                /* otherwise throw an error */
                else
                {
                    PARSER_EMIT_ERR(
                        ERR_SYNTAX,
                        current_token.x,
                        current_token.y,
                        current_token.span,
                        "expected closing ')' after procedure call arguments"
                    );
                    return 0;
                }
            } while(1);
        }

        /*(TEMP)*/ if (current(self).kind != TOK_RPAREN)
        {
            printf("NOT RPAREN\n");
            return 0;
        }

        /* make list node */
        size_t args_end = current(self).span.pos;
        Ast_Node list_base = Node_Build(AST_LIST, (Span) {args_start, args_end - args_start});
        list_base.v_list = args;
        Node_Idx args_idx = Push_And_Get_Id(self->map, list_base);

        /* make call node */
        Span span = (Span) {tk.span.pos, tk.span.len + current(self).span.len};
        Ast_Call_Expr call_expr = (Ast_Call_Expr) {.symbol = expr, .args = args_idx};
        Ast_Node node = Node_Build(AST_CALL_EXPR, span);
        node.v_call_expr = call_expr;

        expr = Push_And_Get_Id(self->map, node);
    }

    return expr;
}

static Node_Idx parse_binary(parser *self)
{
    Token tk = current(self);
    Node_Idx expr = parse_call(self);
    PARSER_CHECK_INVALID_NODE(expr);
    
    Ast_Op_Kind op = Get_Binary_Infix_Operator(peek(self).kind);
    if (op != -1)
    {
        next(self); /* operator */
        next(self); /* beginning of RHS */
        Span span = (Span) {tk.span.pos, tk.span.len + current(self).span.len};
       
        Node_Idx rhs = parse_expr(self);
        PARSER_CHECK_INVALID_NODE(rhs);

        Ast_Binary_Expr value = (Ast_Binary_Expr) {.lhs = expr, .op = op, .rhs = rhs};
        Ast_Node node = Node_Build(AST_BINARY_EXPR, span);
        
        node.v_binary_expr = value;
        expr = Push_And_Get_Id(self->map, node);
    }

    return expr;
}
static Node_Idx parse_assign(parser *self)
{
    Token tk = current(self);
    Node_Idx expr = parse_binary(self);
    PARSER_CHECK_INVALID_NODE(expr);
    
    Ast_Op_Kind op = Get_Assignment_Infix_Operator(peek(self).kind);
    if (op != -1)
    {
        next(self); /* operator */
        next(self); /* beginning of RHS */
        Span span = (Span) {tk.span.pos, tk.span.len + current(self).span.len};
       
        Node_Idx value = parse_expr(self);
        PARSER_CHECK_INVALID_NODE(value);

        Ast_Node node = Node_Build(AST_ASSIGN_EXPR, span);
        node.v_assign_expr = (Ast_Assign_Expr) {
            .name = expr,
            .op = op,
            .value = value
        };
        expr = Push_And_Get_Id(self->map, node);
    }

    return expr;
}

static Node_Idx parse_expr(parser *self)
{
    return parse_assign(self);
}

static Node_Idx parse_variable(parser *self, bool mutability)
{
    Token tk = current(self);
    Node_Idx symbol = parse_literal(self);
    PARSER_CHECK_INVALID_NODE(symbol);
    
    /* validate symbol */
    Ast_Node *symbol_node = List_Get(self->map, symbol);
    if (symbol_node->type != AST_SYMBOL)
    {
        PARSER_EMIT_ERR(ERR_SYNTAX, tk.x, tk.y, tk.span, "expected symbol");
        return 0;
    }

    /* initializer */
    Node_Idx initializer = 0;
    if (peek(self).kind != TOK_NEWLINE)
    {
        /* validate EQ */
        if (peek(self).kind != TOK_EQ)
        {
            Token peeked = peek(self);
            PARSER_EMIT_ERR(ERR_SYNTAX, peeked.x, peeked.y, peeked.span, "expected '='");
            return 0;
        }

        next(self); /* EQ */
        next(self); /* after EQ */
        initializer = parse_expr(self);
        PARSER_CHECK_INVALID_NODE(symbol);
    }

    /* create node */
    Token c = current(self); Span span = SPAN_FROM_THEN_TO_CURRENT(tk, c);
    Ast_Node node = Node_Build(AST_VARIABLE_DECL, span);
    node.v_variable_decl = (Ast_Variable_Decl) {
        .symbol = symbol,
        .initializer = initializer,
        .mutability = mutability
    };
    return Push_And_Get_Id(self->map, node);
}

static Node_Idx parse_statement(parser *self)
{
    Token tk = current(self);
    Node_Idx stmt = 0;

    switch (tk.kind)
    {
        case TOK_LET:
        case TOK_MUT:
        {
            bool mutability = tk.kind == TOK_MUT;
            next(self); /* symbol */

            stmt = parse_variable(self, mutability);
            PARSER_CHECK_INVALID_NODE(stmt);

            break;
        }

        default:
        {
            Node_Idx expr = parse_expr(self);
            PARSER_CHECK_INVALID_NODE(expr);

            size_t x = current(self).x;
            size_t y = current(self).y;
            Ast_Node *expr_node = List_Get(self->map, expr);
            if (expr_node->type != AST_ASSIGN_EXPR
                && expr_node->type != AST_CALL_EXPR)
            {
                PARSER_EMIT_ERR(ERR_SYNTAX, x, y, expr_node->span, "invalid expression statement");
                return 0;
            }

            stmt = expr;
            break;
        };
    }

    if (peek(self).kind != TOK_NEWLINE && peek(self).kind != TOK_EOF)
    {
        Token peeked = peek(self);
        PARSER_EMIT_ERR(ERR_SYNTAX, peeked.x, peeked.y, peeked.span, "expected ';'");
        return 0;
    }

    next(self); /* TOK_NEWLINE */
    return stmt;
}

//===============================================================================//
// TESTS IMPLEMENTATION
//===============================================================================//

static bool test_parser(Test_Info *info, const char *src)
{
    /* tokenize everything */
    List tokens = tokenize(src);
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
        /* parse one stmt */
        Node_Idx stmt = parse_statement(&self);
        
        Report_Errors(&errors, src, "<TestPath>");
        if (stmt == 0)
        {
            info->message = "zero ID";
            info->status = false;
            info->success = false;
            return false;
        }
        
        /* add this expr to the program node */
        Print_Node(&map, stmt, 0);
        List_Add(&root.v_root, &stmt);
        next(&self);

        if (current( &self).kind == TOK_EOF) break;
    } while(1);
    
    info->status = true;
    info->success = true;

    return true;
}

void Test_Parser(Test_Info *info)
{
    {
        const char *src = "mut i = 10\ni = i+1\nprint(i)";
        if (!test_parser(info, src)) return;
    }

    info->success = true;
    info->status = true;
}

void Test_Lexer(Test_Info *info)
{
    const char *src = "+ += == - \nlet mut \nmain";

    List tokens = tokenize(src);
    if (tokens.capacity == 0)
    {
        info->message = "Failed to allocate the tokens list";
        info->status = false;
        info->success = false;
    }

    for (size_t i = 0 ; i < tokens.count; i++)
    {
        Token *t = (Token*)List_Get(&tokens, i);
        Print_Token(src, t);
    }

    List_Free(&tokens);
    info->status = true;
    info->success = true;
}