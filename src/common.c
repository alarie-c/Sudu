#include "common.h"
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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