#include "lexer.h"
#include "common.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#define IS_EOF ((self->pos) >= (self->len))
#define SPAN(len) ((Span) {self->pos, (len)})

//-------------------------------------------------------------------------------//
// token methods
//-------------------------------------------------------------------------------//

Token_Kind cmp_keywords(const char *src, const Span *span)
{
    if (Cmp_Lexeme(src, span, "function"))
        return TOK_FUNCTION;
    else if (Cmp_Lexeme(src, span, "let"))
        return TOK_LET;
    else
        return TOK_SYMBOL;
}

void Print_Token(const char *src, Token const *self)
{
    // size_t y = self->span.y;
    // size_t x = self->span.x;

    size_t pos = self->span.pos;
    size_t len = self->span.len;

    const char *type = TOKEN_KIND_NAMES[self->kind];
    
    // buffer is always big enough for an escape sequence if needed
    size_t size = self->span.len + 1;
    size_t buffer_size = size >= 3 ? size : 3;
    char lex[buffer_size];
    Get_Lexeme(lex, buffer_size, src, &self->span, WITH_ESCAPES);

    if (PRINT_SPAN_INTERNALS)
    {
        printf("%zu:%zu | %zu @ %zu | %s | '%s'\n", self->y, self->x, pos, len, type, lex);
    }
    else
    {
        printf("%s | '%s'\n", type, lex);
    }
}

//-------------------------------------------------------------------------------//
// lexer methods
//-------------------------------------------------------------------------------//

Lexer *Init_Lexer(const char *src)
{
    size_t len = strlen(src);
    Lexer *self = malloc(sizeof(Lexer));
    if (self == NULL) return NULL;

    self->len = len;
    self->src = src;
    self->x = 1;
    self->y = 1;
    self->pos = 0;
    return self;
}

void Free_Lexer(Lexer *self)
{
    if (self == NULL) return;
    free(self);
}

void consume(Lexer *self)
{
    if (IS_EOF)
        return;
    self->pos += 1;
    self->x += 1;
}

char peek(const Lexer *self)
{
    if (self->pos + 1 >= self->len)
        return '\0';
    return self->src[self->pos + 1];
}

char current_char(const Lexer *self)
{
    if (IS_EOF)
        return '\0';
    return self->src[self->pos];
}

void eat_whitespace(Lexer *self)
{
    char ch = current_char(self);
    while (ch == ' ' || ch == '\t' || ch == '\r')
    {
        consume(self);
        ch = current_char(self);
    }
}

//-------------------------------------------------------------------------------//
// lexical scanner 
//-------------------------------------------------------------------------------//

Token Next_Token(Lexer *self)
{

    if (IS_EOF)
    {
        return (Token) {TOK_EOF, SPAN(1), self->x, self->y};
    }
    
    eat_whitespace(self);
    char ch = current_char(self);
    Token tk;

    switch (ch)
    {
        case '\n':
        {
            tk = (Token) {TOK_NEWLINE, SPAN(1), self->x, self->y};
            self->y++;
            self->x = 0; /* zero here because consume() will advance it to 1 */
            break;
        }
        case '(':
        {
            tk = (Token) {TOK_LPAREN, SPAN(1), self->x, self->y};
            break;
        }
        case ')':
        {
            tk = (Token) {TOK_RPAREN, SPAN(1), self->x, self->y};
            break;
        }

        case '+':
        {
            tk = (Token) {TOK_PLUS, SPAN(1), self->x, self->y};
            break;
        }
        case '-':
        {
            tk = (Token) {TOK_MINUS, SPAN(1), self->x, self->y};
            break;
        }
        case '*':
        {
            tk = (Token) {TOK_STAR, SPAN(1), self->x, self->y};
            break;
        }
        case '/':
        {
            tk = (Token) {TOK_SLASH, SPAN(1), self->x, self->y};
            break;
        }
        case '%':
        {
            tk = (Token) {TOK_PERCENT, SPAN(1), self->x, self->y};
            break;
        }
        
        // handle comments
        case '#':
        {
            do {
                if (current_char(self) == '\n'
                    || current_char(self) == '\0')
                {
                    break;
                }
                consume(self);
            } while (true);

            return Next_Token(self);
        }

        default:
        {
            if (isalpha(ch) || ch == '_')
            {
                size_t start = self->pos;
                size_t startx = self->x;

                do {
                    ch = peek(self);
                    if (!isalnum(ch) && ch != '_')
                        break;
                    consume(self);
                } while (true);

                size_t end = self->pos;
                Span span = (Span) {start, end - start + 1};
                Token_Kind kind = cmp_keywords(self->src, &span);
                
                tk = (Token) {kind, span, startx, self->y};
                break; 
            }
            
            if (isdigit(ch))
            {
                Token_Kind kind = TOK_INTEGER;
                size_t start = self->pos;
                size_t startx = self->x;

                do {
                    ch = peek(self);
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
                        consume(self);
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
            
            tk = (Token) {TOK_ILLEGAL, SPAN(1), self->x, self->y};
            break;
        }
    }

    consume(self);
    return tk;
}

//-------------------------------------------------------------------------------//
// tests
//-------------------------------------------------------------------------------//

void Test_Lexer()
{
    printf("[test] lexer\n");
    
    const char *src = "function main\n    end#this is a comment\n";
    printf("src: %s\n", src);
    
    Lexer *lexer = Init_Lexer(src);
    
    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_FUNCTION);
        assert(t.x == 1);
        assert(t.y == 1);
    }

    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_SYMBOL);
        assert(t.x == 10);
        assert(t.y == 1);
    }

    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_NEWLINE);
        assert(t.x == 14);
        assert(t.y == 1);
    }

    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_SYMBOL);
        assert(t.x == 5);
        assert(t.y == 2);
    }

    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_NEWLINE);
        assert(t.x == 26);
        assert(t.y == 2);
    }

    {
        Token t = Next_Token(lexer);
        Print_Token(src, &t);
        assert(t.kind == TOK_EOF);
        assert(t.x == 1);
        assert(t.y == 3);
    }

    printf("[test] lexer test passed\n");
}