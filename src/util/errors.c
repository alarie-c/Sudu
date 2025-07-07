#include "util/errors.h"
#include "util/common.h"
#include "util/tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

//-------------------------------------------------------------------------------//
// reporting methods
//-------------------------------------------------------------------------------//

void Report_Errors(List *errrors, const char *src, const char *path)
{
    if (!errrors || errrors->count == 0) return;
    
    for (size_t i = errrors->count - 1 ;; i--)
    {
        Error *error = (Error *)List_Get(errrors, i);
        if (error) Print_Error(src, path, error);
        if (i == 0) break;
    }
}

void hide_newlines(char *str)
{
    if (str == NULL) return;

    size_t len = strlen(str);
    for (size_t i = 0; i <= len; i++)
    {
        if (str[i] == '\n' || 
           (str[i] == '\0' && i != len))
        {
            str[i] = ' ';
        }
    }
}

struct line_fetch_result { Span span; bool valid; };
struct line_fetch_result fetch_line(const char *src, size_t i)
{
    size_t src_len = strlen(src);
    size_t start = i, end = i;
    
    if (i > src_len)
        return (struct line_fetch_result) {(Span) {0}, false};

    // find the beginning of the line
    for (size_t j = i - 1; /*N/A*/; j--)
    {
        if (j == 0)
        {
            start = 0;
            break;
        }
        if (src[j] == '\n' && j != i)
        {
            start = j + 1;
            break;
        }
    }

    if (i == src_len)
    {
        return (struct line_fetch_result) {(Span) {start, src_len - start}, true};
    }

    // find the end of the line
    for (size_t j = i; j <= src_len; j++)
    {
        if ((src[j] == '\n' || src[j] == '\0'))
        {
            end = j;
            break;
        }
    }

    assert(end <= src_len); /* returning the \0 of the source is irrelevant */
                            /* because that position will just be overwritte with */
                            /* a \0 when we get the lexeme. */
    assert(start < src_len);
    assert(end >= i);
    assert(start <= i);

    // size_t len = (end - start > 0) ? end - start : 1;
    size_t len = end - start + 1;
    return (struct line_fetch_result) {(Span) {start, len}, true};
}

//-------------------------------------------------------------------------------//
// print methods
//-------------------------------------------------------------------------------//

void print_invalid_return(Error const *self)
{
    const char *src = self->data_invalid_return.def_src;
    const char *path = self->data_invalid_return.def_path;
    Span span = self->data_invalid_return.def_span;
    size_t y = self->data_invalid_return.def_y;
    size_t x = self->data_invalid_return.def_x;

    printf("%s%sdefinition:%s %s:%zu:%zu\n",
        TERM_ESC, TERM_MAGENTAB, TERM_RESET, path, y, x);

    struct line_fetch_result line = fetch_line(src, span.pos);
    if (!line.valid)
    {
        printf("<Error fetching line content>");
        return;
    }
    
    char *raw = Get_Lexeme(src, line.span.pos, line.span.len); 
    hide_newlines(raw);
    printf("  |\n  | %s\n  | %s%s", raw, TERM_ESC, TERM_RED);
    
    /* Line Start   */ size_t l_start = line.span.pos; 
    /* Line End     */ size_t l_end = line.span.pos + line.span.len - 1; 
    /* Carets Start */ size_t c_start = span.pos; 
    /* Carets End   */ size_t c_end = span.pos + span.len - 1; 
    
    for (size_t i = l_start; i <= l_end; i++)
    {
        if (i >= c_start && i <= c_end)
            putchar('^');
        else
            putchar(' ');
    }
    printf("%s\n", TERM_RESET);
    printf("function defined to return type '%s'. Either change the return expression or use a runtime cast.\n",
        self->data_invalid_return.def_type
    );
}

void Print_Error(const char *src, const char *path, Error const *self)
{
    printf("%s%serror:%s %s:%zu:%zu",
        TERM_ESC, TERM_REDB, TERM_RESET, path, self->y, self->x);

    const char *type = ERROR_TYPE_NAMES[self->type];
    printf("%s%s %s %s\n", TERM_ESC, TERM_YELLOWB, type, TERM_RESET);

    struct line_fetch_result line = fetch_line(src, self->span.pos);
    if (!line.valid)
    {
        printf("<Error fetching line content>\n");
        return;
    }

    char *raw = Get_Lexeme(src, line.span.pos, line.span.len); 
    hide_newlines(raw);
    printf("  |\n  | %s\n  | %s%s", raw, TERM_ESC, TERM_RED);
    
    /* Line Start   */ size_t l_start = line.span.pos; 
    /* Line End     */ size_t l_end = line.span.pos + line.span.len - 1; 
    /* Carets Start */ size_t c_start = self->span.pos; 
    /* Carets End   */ size_t c_end = self->span.pos + self->span.len - 1; 
    
    for (size_t i = l_start; i <= l_end; i++)
    {
        if (i >= c_start && i <= c_end)
            putchar('^');
        else
            putchar(' ');
    }

    printf("%s\n%s\n", TERM_RESET, self->message);
    
    switch (self->type)
    {
        case ERR_INVALID_RETURN:
        {
            print_invalid_return(self);
            break;
        }

        default: break;
    }
    printf("\n");
}

//-------------------------------------------------------------------------------//
// error methods
//-------------------------------------------------------------------------------//

void Append_Invalid_Return(Error *self, Error_Invalid_Return data)
{
    if (self->type != ERR_INVALID_RETURN) return;
    self->data_invalid_return = data;
}

Error Make_Error(Error_Type type, size_t x, size_t y, Span span, const char *msg)
{
    return (Error) {
        .type = type,
        .x = x, .y = y,
        .span = span,
        .message = msg,
        .msg_len = strlen(msg)
    };
}

void Free_Error(Error *self)
{
}

//-------------------------------------------------------------------------------//
// tests
//-------------------------------------------------------------------------------//

void Test_Error_Collection(Test_Info *info)
{
    const char *path = "test.sudu";
    const char *src = "function main()\n"
                      "    let 2 = 20\n"
                      "    return 50\n"
                      "end\n";

    printf("source:\n'%s'\n", src);

    assert(src[24] == '2');
    assert(src[35] == 'r');

    List ec = List_New(sizeof(Error), INIT_ERROR_CAPACITY);
    
    {
        Span span = (Span) {24, 1};
        Error err = Make_Error(ERR_SYNTAX, 33, 2, span, "expected identifier, got '2'");
        List_Add(&ec, &err);
    }
    {
        Span span = (Span) {35, 9};
        Error err = Make_Error(ERR_SYNTAX, 5, 3, span, "function 'main' is annoted to return nothing, but returns 'int' here");
        List_Add(&ec, &err);
    }

    Report_Errors(&ec, src, path);
    List_Free(&ec);
    
    info->status = true;
    info->success = true;
}

void Test_Append_Data_Errors(Test_Info *info)
{
    const char *path = "test.sudu";
    const char *src = "function main() -> integer\n"
                      "    return 0.5\n"
                      "end\n";
    assert(src[31] == 'r');
    assert(src[19] == 'i');

    List ec = List_New(sizeof(Error), INIT_ERROR_CAPACITY);

    {
        Span span = (Span) {31, 10};
        Error err = Make_Error(ERR_INVALID_RETURN, 33, 2, span, "cannot return type 'float' from function defined to return type 'integer'");
        Error_Invalid_Return data = (Error_Invalid_Return){
            .def_x = 0,
            .def_y = 0,
            .def_path = "test.sudu",
            .def_span = (Span) {19, 7},
            .def_type = "integer",
            .def_src = src
        };
        Append_Invalid_Return(&err, data);
        List_Add(&ec, &err);
    }

    Report_Errors(&ec, src, path);
    List_Free(&ec);

    info->status = true;
    info->success = true;
}