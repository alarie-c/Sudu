#include "frontend/lexer.h"
#include "util/tests.h"
#include "util/common.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//-------------------------------------------------------------------------------//
// private macros
//-------------------------------------------------------------------------------//

#define IS_EOF ((lexer->pos) >= (lexer->len))
#define SPAN(len) ((Span) {lexer->pos, (len)})
#define SPAN_DOUBLE ((Span) {lexer->pos - 1, 2})

//-------------------------------------------------------------------------------//
// lexer internal state
//-------------------------------------------------------------------------------//

typedef struct
{
    const char *src;
    size_t len;
    size_t pos;
    size_t x;
    size_t y;
} state;

//-------------------------------------------------------------------------------//
// lexer helper methods
//-------------------------------------------------------------------------------//

static Token_Kind cmp_keywords(const char *src, const Span *span)
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

static void consume(state *lexer)
{
    if (IS_EOF)
        return;
    lexer->pos += 1;
    lexer->x += 1;
}

static char peek(const state *lexer)
{
    if (lexer->pos + 1 >= lexer->len)
        return '\0';
    return lexer->src[lexer->pos + 1];
}

static char current_char(const state *lexer)
{
    if (IS_EOF)
        return '\0';
    return lexer->src[lexer->pos];
}

static void eat_whitespace(state *lexer)
{
    char ch = current_char(lexer);
    while (ch == ' ' || ch == '\t' || ch == '\r')
    {
        consume(lexer);
        ch = current_char(lexer);
    }
}

//-------------------------------------------------------------------------------//
// tokenizer function
//-------------------------------------------------------------------------------//

static Token next_token(state *lexer)
{

    if (IS_EOF)
    {
        return (Token) {TOK_EOF, SPAN(1), lexer->x, lexer->y};
    }
    
    eat_whitespace(lexer);
    char ch = current_char(lexer);
    size_t start = lexer->x;
    Token tk;

    switch (ch)
    {
        case '\n':
        {
            tk = (Token) {TOK_NEWLINE, SPAN(1), lexer->x, lexer->y};
            lexer->y++;
            lexer->x = 0; /* zero here because consume() will advance it to 1 */
            break;
        }
        case '(':
        {
            tk = (Token) {TOK_LPAREN, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case ')':
        {
            tk = (Token) {TOK_RPAREN, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '=':
        {
            if (peek(lexer) == '=')
            {
                consume(lexer);
                tk = (Token) {TOK_EQ_EQ, SPAN_DOUBLE, start, lexer->y};
                break;
            }
            tk = (Token) {TOK_EQ, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '+':
        {
            if (peek(lexer) == '=')
            {
                consume(lexer);
                tk = (Token) {TOK_PLUS_EQ, SPAN_DOUBLE, start, lexer->y};
                break;
            }
            tk = (Token) {TOK_PLUS, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '-':
        {
            tk = (Token) {TOK_MINUS, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '*':
        {
            tk = (Token) {TOK_STAR, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '/':
        {
            tk = (Token) {TOK_SLASH, SPAN(1), lexer->x, lexer->y};
            break;
        }
        case '%':
        {
            tk = (Token) {TOK_PERCENT, SPAN(1), lexer->x, lexer->y};
            break;
        }
        
        // handle comments
        case '#':
        {
            do {
                if (current_char(lexer) == '\n'
                    || current_char(lexer) == '\0')
                {
                    break;
                }
                consume(lexer);
            } while (true);

            return next_token(lexer);
        }

        default:
        {
            if (isalpha(ch) || ch == '_')
            {
                size_t start = lexer->pos;
                size_t startx = lexer->x;

                do {
                    ch = peek(lexer);
                    if (!isalnum(ch) && ch != '_')
                        break;
                    consume(lexer);
                } while (true);

                size_t end = lexer->pos;
                Span span = (Span) {start, end - start + 1};
                Token_Kind kind = cmp_keywords(lexer->src, &span);
                
                tk = (Token) {kind, span, startx, lexer->y};
                break; 
            }
            
            if (isdigit(ch))
            {
                Token_Kind kind = TOK_INTEGER;
                size_t start = lexer->pos;
                size_t startx = lexer->x;

                do {
                    ch = peek(lexer);
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
                        consume(lexer);
                    }
                    else
                    {
                        break;
                    }
                } while (true);

                size_t end = lexer->pos;
                Span span = (Span) {start, end - start + 1};
                tk = (Token) {kind, span, startx, lexer->y};
                break; 
            }
            
            tk = (Token) {TOK_ILLEGAL, SPAN(1), lexer->x, lexer->y};
            break;
        }
    }

    consume(lexer);
    return tk;
}

//-------------------------------------------------------------------------------//
// tokenizer interface
//-------------------------------------------------------------------------------//

List Tokenize(const char *src)
{
    List tokens = List_New(sizeof(Token), INIT_TOKEN_CAPACITY);
    if (tokens.capacity == 0) return (List) {0};

    /* create the state struct */
    state lexer = (state) { 
        .src = src,
        .len = strlen(src),
        .pos = 0,
        .x = 1, .y = 1
    };

    do {
        Token next = next_token(&lexer);
        List_Add(&tokens, &next);
        
        if (next.kind == TOK_EOF) break;
    } while(1);

    return tokens;
}

//-------------------------------------------------------------------------------//
// tests
//-------------------------------------------------------------------------------//

void Test_Lexer(Test_Info *info)
{
    const char *src = "+ += == - \nlet mut \nmain";

    List tokens = Tokenize(src);
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