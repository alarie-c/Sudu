#include "util/common.h"
#include "util/tests.h"
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//-------------------------------------------------------------------------------//
// tokens methods
//-------------------------------------------------------------------------------//

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
// list methods
//-------------------------------------------------------------------------------//

List List_New(size_t size, size_t init_capacity)
{
    void *data = malloc(init_capacity * size);
    if (!data) return (List) {0};

    return (List) {
        .data = data,
        .size = size,
        .capacity = init_capacity,
        .count = 0
    };
}

void *List_Get(List *self, size_t n)
{
    if (!self || n >= self->count)
        return NULL;

    void *item = (char *)self->data + n * self->size;
    return item;
}

void List_Add(List *self, void* element)
{
    if (!self || !element) return;

    /* grow array if neccesary */
    if (self->count >= self->capacity)
    {
        size_t new_capacity = (self->capacity != 0)
                            ? (self->capacity * LIST_GROWTH_FACTOR)
                            : 4;
        void *new_data = realloc(self->data, new_capacity * self->size);
        if (!new_data) return;

        self->data = new_data;
        self->capacity = new_capacity;
    }

    /* deep copy the data into the array */
    void *dest = (char *)self->data + self->count * self->size;
    memcpy(dest, element, self->size);
    self->count++;
}

void List_Free(List *self)
{
    if (!self) return;
    free(self->data);
}

//-------------------------------------------------------------------------------//
// lexeme methods
//-------------------------------------------------------------------------------//

size_t Lexeme_Buffer_Len(Span const *span)
{
    size_t size = span->len + 1; /* plus one for the null terminator */
    return size >= 3 ? size : 3;
}

void Get_Lexeme(char *buf, size_t buf_size, const char *src, Span const *span, bool escapes)
{
    if (!src || !buf) {
        snprintf(buf, buf_size, "\\0");
        return;
    }

    size_t pos = span->pos;
    size_t len = span->len;
    size_t max_len = buf_size - 1;
    if (len > max_len) len = max_len;

    memcpy(buf, src + pos, len);
    buf[len] = '\0';

    if (escapes == false) return;

    if (strncmp(buf, "\n", 2) == 0)
        snprintf(buf, buf_size, "\\n");
    else if (strncmp(buf, "\0", 2) == 0)
        snprintf(buf, buf_size, "\\0");
}

bool Cmp_Lexeme(const char *src, const Span *span, const char *lit)
{
    return strncmp(src + span->pos, lit, span->len) == 0
           && lit[span->len] == '\0';
}

void Remove_Underbars(char *str)
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
// tests
//-------------------------------------------------------------------------------//

void Test_List(Test_Info *info)
{
    printf("> Testing List<int>\n");

    List int_list = List_New(sizeof(int), 4);
   
    int head = 0;
    List_Add(&int_list, &head);

    /* add to the list */
    for (int i = 1; i < 50; i++)
    {
        int val = i;
        List_Add(&int_list, &val);
        
        int *last = (int *)List_Get(&int_list, i - 1);
        if (!last)
        {
            info->success = false;
            info->status = false;
            info->message = "Got NULL from List_Get() in int loop!";
            return;
        }

        if (*last != i - 1) {
            info->success = false;
            info->status = false;
            info->message = "List value did not match expected";
            return;
        }

        printf("> '%i'\n", *last + i);
    }

    List_Free(&int_list);

    info->success = true;
    info->status = true;
}