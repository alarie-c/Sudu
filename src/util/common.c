#include "util/common.h"
#include "util/tests.h"
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//===============================================================================//
// TOKEN IMPLEMENTATION
//===============================================================================//

void Print_Token(const char *src, Token const *self)
{
    size_t pos = self->span.pos;
    size_t len = self->span.len;
    const char *type = TOKEN_KIND_NAMES[self->kind];
    const char *lex = Get_Lexeme(src, self->span.pos, self->span.len);
    
    if (self->kind == TOK_NEWLINE)
    {
        free((void *)lex);
        lex = "\\n";
    }
       
    printf("%zu:%zu | %zu+%zu | %s | '%s'\n", self->y, self->x, pos, len, type, lex);   

    if (self->kind != TOK_NEWLINE)
        free((void *)lex);
}

//===============================================================================//
// LIST IMPLEMENTATION
//===============================================================================//

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

    void *dest = (char *)self->data + self->count * self->size;
    memcpy(dest, element, self->size);
    self->count++;
}

void List_Free(List *self)
{
    if (!self) return;
    free(self->data);
}

//===============================================================================//
// LEXEME FUNCTIONS
//===============================================================================//

char *Get_Lexeme(const char *src, size_t pos, size_t len)
{
    if (!src) return "\0";

    /* allocate the string dynamically */
    char *string = malloc(len + 1);
    if (!string) return "\0";

    memcpy(string, src + pos, len);
    string[len] = '\0';

    return string;
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

//===============================================================================//
// OPERATOR HELPER FUNCTIONS
//===============================================================================//

static const int OPERATOR_FLAGS[] = {
    #define X(name, token, str, flag) [name] = flag,
    OPERATOR_LIST
    #undef X
};

static inline int get_op_by_token(Token_Kind kind)
{
    switch (kind) {
        #define X(name, token, str, flag) case token: return name;
        OPERATOR_LIST
        #undef X
        default: return -1;
    }
}

/// @brief Returns the `Operator` pertaining to this token kind that is a binary operator.
inline int Get_Binary_Operator(Token_Kind kind)
{
    int op = get_op_by_token(kind);
    return (op != -1 && (OPERATOR_FLAGS[op] & OP_FLAG_BINARY) ? op : -1);
}

/// @brief Returns the `Operator` pertaining to this token kind that is a unary operator.
inline int Get_Unary_Operator(Token_Kind kind)
{
    int op = get_op_by_token(kind);
    return (op != -1 && (OPERATOR_FLAGS[op] & OP_FLAG_UNARY) ? op : -1);
}

/// @brief Returns the `Operator` pertaining to this token kind that is an assignment operator.
inline int Get_Assign_Operator(Token_Kind kind)
{
    int op = get_op_by_token(kind);
    return (op != -1 && (OPERATOR_FLAGS[op] & OP_FLAG_ASSIGN) ? op : -1);
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