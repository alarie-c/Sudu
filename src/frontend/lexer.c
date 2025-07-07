#include "util/common.h"
#include "util/errors.h"
#include "frontend/lexer.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

//===============================================================================//
// LEXER MACROS
//===============================================================================//

#define IS_EOF ((lexer->pos) >= (lexer->len))
#define TOKEN_HERE(kind) (Token) {(kind), ((Span) {lexer->pos, 1}), lexer->x, lexer->y}
#define TOKEN_DOUBLE(kind) (Token) {(kind), ((Span) {start_pos, 2}), start_col, lexer->y}

//===============================================================================//
// LEXER IMPLEMENTATION
//===============================================================================//

typedef struct _lexer_t
{
    List *errs;
    const char *src;
    size_t len;
    size_t pos;
    size_t x;
    size_t y;
} lexer_t;

static Token_Kind cmp_keywords(const char *src, const Span *span)
{
    if      (Cmp_Lexeme(src, span, "func"))
        return TOK_FUNC;
    else if (Cmp_Lexeme(src, span, "let"))
        return TOK_LET;
    else if (Cmp_Lexeme(src, span, "var"))
        return TOK_VAR;
    else
        return TOK_SYMBOL_LITERAL;
}

static void consume_char(lexer_t *lexer)
{
    if (IS_EOF)
        return;
    lexer->pos += 1;
    lexer->x += 1;
}

static char peek_char(const lexer_t *lexer)
{
    if (lexer->pos + 1 >= lexer->len)
        return '\0';
    return lexer->src[lexer->pos + 1];
}

static bool expect_char(lexer_t *lexer, char c)
{
    if (peek_char(lexer) == c)
    {
        lexer->pos++;
        lexer->x++;
        return true;
    }
    return false;
}

static char current_char(const lexer_t *lexer)
{
    if (IS_EOF)
        return '\0';
    return lexer->src[lexer->pos];
}

static void eat_whitespace(lexer_t *lexer)
{
    char ch = current_char(lexer);
    while (ch == ' ' || ch == '\t' || ch == '\r')
    {
        consume_char(lexer);
        ch = current_char(lexer);
    }
}

static bool is_symbol_char_start(char c)
{
    return (c != '\0' && (isalpha(c) || c == '_'));
}

static bool is_symbol_char(char c)
{
    return (c != '\0' && (isalnum(c) || c == '_'));
}

static bool is_number_char(char c)
{
    return (c != '\0' && (isdigit(c) || c == '_'));
}

static bool breaks_comment(char c)
{
    return (c == '\n' || c == '\0');
}

static Span distance_span(size_t to, size_t from)
{
    return (Span) {from, to - from + 1};
}

static char peek_n_ahead(const lexer_t *lexer, size_t n)
{
    if (lexer->pos + n >= lexer->len)
        return '\0';
    return lexer->src[lexer->pos + n];
}

//===============================================================================//
// TOKENIZER
//===============================================================================//

static Token next_token(lexer_t *lexer)
{

    if (IS_EOF) return TOKEN_HERE(TOK_EOF);
    
    eat_whitespace(lexer);
    char ch = current_char(lexer);
    size_t start_col = lexer->x;
    size_t start_pos = lexer->pos;

    switch (ch)
    {
        case '(': return TOKEN_HERE(TOK_OPEN_PAREN);
        case ')': return TOKEN_HERE(TOK_CLOSE_PAREN);
        case '{': return TOKEN_HERE(TOK_OPEN_BRACE);
        case '}': return TOKEN_HERE(TOK_CLOSE_BRACE);
        case '[': return TOKEN_HERE(TOK_OPEN_BRACKET);
        case ']': return TOKEN_HERE(TOK_CLOSE_BRACKET);
        case '%': return TOKEN_HERE(TOK_PERCENT);
        case ':': return TOKEN_HERE(TOK_COLON);
        case ';': return TOKEN_HERE(TOK_SEMICOLON);
        case '.': return TOKEN_HERE(TOK_DOT);
        case ',': return TOKEN_HERE(TOK_COMMA);
        case '?': return TOKEN_HERE(TOK_QUESTION);

        case '\n':
        {
            Token tk = TOKEN_HERE(TOK_NEWLINE);
            lexer->y++;
            lexer->x = 0; /* zero here because consume() will advance it to 1 */
            return tk;
        }

        case '+':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_PLUS_EQUALS);
            if (expect_char(lexer, '+'))
                return TOKEN_DOUBLE(TOK_PLUS_PLUS);
            return TOKEN_HERE(TOK_PLUS);
        }

        case '-':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_MINUS_EQUALS);
            if (expect_char(lexer, '-'))
                return TOKEN_DOUBLE(TOK_MINUS_MINUS);
            if (expect_char(lexer, '>'))
                return TOKEN_DOUBLE(TOK_ARROW);
            return TOKEN_HERE(TOK_MINUS);
        }

        case '*':
        {
            if (expect_char(lexer, '*'))
            {
                if (expect_char(lexer, '='))
                    return (Token) {
                        .kind = TOK_STAR_STAR_EQUALS,
                        .span = (Span) {start_pos, 3},
                        .x = start_col,
                        .y = lexer->y
                    };
                return TOKEN_DOUBLE(TOK_STAR_STAR);
            }
            
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_STAR_EQUALS);
            return TOKEN_HERE(TOK_STAR);
        }

        case '/':
        {
            if (expect_char(lexer, '/'))
            {
                if (expect_char(lexer, '='))
                    return (Token) {
                        .kind = TOK_SLASH_SLASH_EQUALS,
                        .span = (Span) {start_pos, 3},
                        .x = start_col,
                        .y = lexer->y
                    };
                return TOKEN_DOUBLE(TOK_SLASH_SLASH);
            }
            
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_SLASH_EQUALS);
            return TOKEN_HERE(TOK_SLASH);
        }
     
        case '=':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_EQUALS_EQUALS);
            if (expect_char(lexer, '>'))
                return TOKEN_DOUBLE(TOK_FAT_ARROW);
            return TOKEN_HERE(TOK_EQUALS);
        }

        case '<':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_LESS_EQUALS);
            return TOKEN_HERE(TOK_LESS);
        }

        case '>':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_GREATER_EQUALS);
            return TOKEN_HERE(TOK_GREATER);
        }

        case '!':
        {
            if (expect_char(lexer, '='))
                return TOKEN_DOUBLE(TOK_BANG_EQUALS);
            return TOKEN_HERE(TOK_BANG);
        }

        case '&':
        {
            if (expect_char(lexer, '&'))
                return TOKEN_DOUBLE(TOK_AMPSAND_AMPSAND);
            return TOKEN_HERE(TOK_AMPSAND);
        }

        case '|':
        {
            if (expect_char(lexer, '|'))
                return TOKEN_DOUBLE(TOK_PIPE_PIPE);
            return TOKEN_HERE(TOK_PIPE);
        }


        // handle comments
        case '#':
        {
            do {
                consume_char(lexer);
            } while (!breaks_comment(peek_char(lexer)));
            return next_token(lexer);
        }

        default:
        {
            /* handle identifiers */
            if (is_symbol_char_start(ch))
            {
                while (is_symbol_char(peek_char(lexer)))
                    consume_char(lexer);
                
                Span span = distance_span(lexer->pos, start_pos);
                Token_Kind kind = cmp_keywords(lexer->src, &span);
                return (Token) {
                    .kind = kind,
                    .span = span,
                    .x = start_col,
                    .y = lexer->y,
                };
            }

            /* handle digits */
            if (is_number_char(ch))
            {
                Token_Kind kind = TOK_INTEGER_LITERAL;
                
                char peeked_char;
                while ((peeked_char = peek_char(lexer)) != '\0'
                    && (is_number_char(peeked_char)))
                {
                    consume_char(lexer);
                }

                char maybe_dot = peek_char(lexer);
                char after_dot = (maybe_dot != '\0') ? peek_n_ahead(lexer, 2) : '\0';
                
                if (maybe_dot == '.' && isdigit(after_dot)) {
                    kind = TOK_FLOAT_LITERAL;
                    consume_char(lexer); /* consume the dot */
                    
                    while ((peeked_char = peek_char(lexer)) != '\0'
                        && (is_number_char(peeked_char)))
                    {
                        consume_char(lexer);
                    }
                }

                Span span = distance_span(lexer->pos, start_pos);
                return (Token) {
                    .kind = kind,
                    .span = span,
                    .x = start_col,
                    .y = lexer->y,
                };
            }
            
            return TOKEN_HERE(TOK_ILLEGAL);
        }
    }
}

Tokens Tokenize(const char *src, List *errors)
{
    lexer_t lexer = (lexer_t) {
        .errs = errors,
        .src = src,
        .len = strlen(src),
        .pos = 0,
        .x = 1,
        .y = 1,
    };

    List raw_tokens = List_New(sizeof(Token), INIT_TOKEN_CAPACITY);
    if (raw_tokens.capacity == 0)
        return (Tokens) {
            .tokens = raw_tokens,
            .valid = false,
        };

    Tokens buffer = (Tokens) {
        .tokens = raw_tokens,
        .valid = true,
    };

    while(current_char(&lexer) != '\0')
    {
        Token t = next_token(&lexer);
        List_Add(&buffer.tokens, &t);

        /* throw error if illegal character is found */
        if (t.kind == TOK_ILLEGAL)
        {
            Error e = (Error) {
                .type = ERR_ILLEGAL_CHAR,
                .span = t.span,
                .x = t.x,
                .y = t.y,
                .message = "illegal character in source code",
                .msg_len = strlen("illegal character in source code")
            };
            List_Add(errors, &e);
            buffer.valid = false;
        }

        consume_char(&lexer);
    }

    Token eof_tok = (Token) {
        .kind = TOK_EOF,
        .span = (Span) {lexer.pos, 1},
        .x = lexer.x,
        .y = lexer.y,
    };
    List_Add(&buffer.tokens, &eof_tok);
    return buffer;
}

void Test_Lexer(Test_Info *info)
{
    const char *src = 
        "( ) { } [ ] \n"
        "+ - * / ++ -- ** += -= *= **= // /= //= % ! != = == < <= > >= | || & && : ; . ? , -> =>";

    List errors = {0};
    Tokens buf = Tokenize(src, &errors);

    if (!buf.valid) {
        Report_Errors(&errors, src, "test");
        Assert(false, info, "lexer returned invalid buffer");
        List_Free(&errors);
        return;
    }

    size_t count = buf.tokens.count;
    Assert(count > 0, info, "lexer produced no tokens");

    Token *tokens = (Token *)buf.tokens.data;

    Assert(tokens[0].kind == TOK_OPEN_PAREN, info, "expected TOK_OPEN_PAREN as first token");
    Assert(tokens[1].kind == TOK_CLOSE_PAREN, info, "expected TOK_CLOSE_PAREN as second token");
    Assert(tokens[2].kind == TOK_OPEN_BRACE, info, "expected TOK_OPEN_BRACE as third token");
    Assert(tokens[3].kind == TOK_CLOSE_BRACE, info, "expected TOK_CLOSE_BRACE as fourth token");

    for (size_t i = 0; i < count; ++i) {
        Token *tok = &tokens[i];
        Print_Token(src, tok);
    }

    List_Free(&errors);

    info->success = true;
    info->status = true;
}

void Test_Literals(Test_Info *info)
{
    const char *src =
        "0 .5 5. 0.5.round() 1_000 1_000.5 42__42 3..14\n"
        "symbol func var let another_symbol343 _and_another";

    List errors = {0};
    Tokens buf = Tokenize(src, &errors);

    if (!buf.valid) {
        Report_Errors(&errors, src, "test");
        Assert(false, info, "lexer returned invalid buffer");
        List_Free(&errors);
        return;
    }

    size_t count = buf.tokens.count;
    Assert(count > 0, info, "lexer produced no tokens");

    Token *tokens = (Token *)buf.tokens.data;

    for (size_t i = 0; i < count; ++i) {
        Token *tok = &tokens[i];
        Print_Token(src, tok);
    }

    List_Free(&errors);

    info->success = true;
    info->status = true;
}

    
