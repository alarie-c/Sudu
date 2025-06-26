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
#include <stddef.h>
#include <common.h>

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

typedef struct
{
    const char *src;
    char *path; /* this is strdup'd and freed later */
    Error *errors;
    size_t size;
    size_t capacity;
} Error_Collection;

Error_Collection *Init_Error_Collection(const char *src, const char *path);
void Ec_Push(Error_Collection *self, Error err);
void Ec_Report_All(Error_Collection const *self);
void Ec_Free(Error_Collection *self);

/* Tests */
void Test_Error_Printing();
void Test_Terminal_Color_Codes();
void Test_Error_Collection();
void Test_Append_Data_Errors();

#endif // ERRORS_H
