#ifndef ERRORS_H
#define ERRORS_H
#include "util/tests.h"
#include "util/common.h"
#include <stddef.h>

//===============================================================================//
// TERMINAL COLOR CODES
//===============================================================================//

#define TERM_ESC "\x1b"
#define TERM_RESET "\x1b[0m"
#define TERM_REDB "[31;1m"
#define TERM_YELLOWB "[33;1m"
#define TERM_GREEN "[32m"
#define TERM_RED "[31m"
#define TERM_MAGENTAB "[35;1m"

//===============================================================================//
// ERROR SUBTYPES
//===============================================================================//

/// @brief Holds location info for the definition of a function
/// so it can report what the type was originally defined as
typedef struct _Error_Invalid_Return
{
    Span def_span;
    size_t def_x;
    size_t def_y;
    const char *def_type;
    const char *def_path;
    const char *def_src;
} Error_Invalid_Return;

//===============================================================================//
// ERROR IMPLEMENTATION
//===============================================================================//

#define INIT_ERROR_CAPACITY 64

typedef enum _Error_Type
{
    ERR_SYNTAX,
    ERR_ILLEGAL_CHAR,
    ERR_UNTERMINATED_LITERAL,
    ERR_EXPECTED_EXPRESSION,
    ERR_INVALID_LITERAL,
    ERR_INVALID_RETURN,
} Error_Type;

static const char* ERROR_TYPE_NAMES[] = {
    "syntax error",
    "illegal character",
    "unterminated literal",
    "expected expression",
    "invalid literal",
    "invalid return type",
};

typedef struct _Error
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

/// @brief Creates a fully allocated error from the information provided.
/// @param type the type of error.
/// @param x the column.
/// @param y the line.
/// @param span the span of source code in question.
/// @param msg the help message to be displayed to the end user.
/// @return error instance.
Error Make_Error(Error_Type type, size_t x, size_t y, Span const span, const char *msg);

/// @brief Appends the `Error_Invalid_Return` subtype struct to the error. Will
/// early return of the `self` parameter is not of type `ERR_INVALID_RETURN`.
/// @param self the error instance.
/// @param data the invalid return data to append.
void Append_Invalid_Return(Error *self, Error_Invalid_Return data);

void Print_Error(const char *src, const char *path, Error const *self);
void Free_Error(Error *self);
void Report_Errors(List *errors, const char *src, const char *path);

//===============================================================================//
// TESTS
//===============================================================================//

void Test_Error_Collection(Test_Info *info);
void Test_Append_Data_Errors(Test_Info *info);

#endif // ERRORS_H
