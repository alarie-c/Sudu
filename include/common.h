#ifndef COMMON_H
#define COMMON_H
#define WITH_ESCAPES true
#define NO_ESCAPES false
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t pos;
    size_t len;
} Span;

size_t Lexeme_Buffer_Len(Span const *span);
void Get_Lexeme(char *buf, size_t buf_size, const char *src, Span const *span, bool escapes);
bool Cmp_Lexeme(const char *src, const Span *span, const char *lit);

#endif // COMMON_H