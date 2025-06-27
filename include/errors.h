#ifndef ERRORS_H
#define ERRORS_H
#define TERM_ESC "\x1b"
#define TERM_RESET "\x1b[0m"
#define TERM_REDB "[31;1m"
#define TERM_YELLOWB "[33;1m"
#define TERM_GREEN "[32m"
#define TERM_RED "[31m"
#define TERM_MAGENTAB "[35;1m"
#define INIT_ERROR_CAPACITY 64
#include "tests.h"
#include <stddef.h>
#include <common.h>

//-------------------------------------------------------------------------------//
// error implementation
//-------------------------------------------------------------------------------//

typedef enum
{
    ERR_SYNTAX,
    ERR_ILLEGAL_CHAR,
    ERR_UNTERMINATED_LITERAL,
    ERR_EXPECTED_EXPRESSION,
    ERR_INVALID_RETURN,
} Error_Type;

static const char* ERROR_TYPE_NAMES[] = {
    "syntax error",
    "illegal character",
    "unterminated literal",
    "expected expression",
    "invalid return type",
};

/// @brief Holds location info for the definition of a function
/// so it can report what the type was originally defined as
typedef struct
{
    Span def_span;
    size_t def_x;
    size_t def_y;
    const char *def_type;
    const char *def_path;
    const char *def_src;
} Error_Invalid_Return;

typedef struct
{
    Error_Type type;
    size_t x;
    size_t y;
    Span span;
    union {
        Error_Invalid_Return data_invalid_return;
    };
    const char *message;
    size_t msg_len;
} Error;

Error Make_Error(Error_Type type, size_t x, size_t y, Span const span, const char *msg);
void Append_Invalid_Return(Error *self, Error_Invalid_Return data);
void Print_Error(const char *src, const char *path, Error const *self);
void Free_Error(Error *self);
void Report_Errors(List *errors, const char *src, const char *path);

/* Tests */
void Test_Error_Collection(Test_Info *info);
void Test_Append_Data_Errors(Test_Info *info);

#endif // ERRORS_H
