#ifndef COMMON_H
#define COMMON_H
#define WITH_ESCAPES true
#define NO_ESCAPES false
#define LIST_GROWTH_FACTOR 2
#include "tests.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    void *data;
    size_t size;
    size_t count;
    size_t capacity;
} List;

List List_New(size_t size, size_t init_capacity);
void *List_Get(List *self, size_t n);
void List_Add(List *self, void* element);
void List_Free(List *self);

typedef struct
{
    size_t pos;
    size_t len;
} Span;

size_t Lexeme_Buffer_Len(Span const *span);
void Get_Lexeme(char *buf, size_t buf_size, const char *src, Span const *span, bool escapes);
bool Cmp_Lexeme(const char *src, const Span *span, const char *lit);
void Remove_Underbars(char *str);

/* List Tests */
void Test_List(Test_Info *info);

#endif // COMMON_H
