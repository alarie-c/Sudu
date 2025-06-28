#ifndef TESTS_H
#define TESTS_H
#define T_TERM_RESET "\x1b[0m"
#define T_TERM_CYANB "\x1b[36;1m"
#define T_TERM_GREENB "\x1b[32;1m"
#define T_TERM_REDB "\x1b[31;1m"
#define T_TERM_YELLOWI "\x1b[33;3m"
#define TEST_TYPE_ASSERTION 0
#define TEST_TYPE_MANUAL 1
#define INIT_TEST_CAPACITY 8
#include <stdbool.h>
#include <stdio.h>

//-------------------------------------------------------------------------------//
// forward declarations
//-------------------------------------------------------------------------------//

/// FWD DECLARATION
typedef struct Test_Info Test_Info;
typedef struct Test_Environment Test_Environment;

/// FWD DECLARATION
/// @brief This is a function pointer that refers to a function
/// that returns void and takes a Test_Info parameter. The test will
/// update the values in this parameter according to how it runs.
typedef void (*Test_Procedure)(Test_Info*);

//-------------------------------------------------------------------------------//
// definitions
//-------------------------------------------------------------------------------//

/// DEFINITION
/// @brief A container for a single test, including all the relevant fields
struct Test_Info
{
    bool status;
    bool success;
    const char *message;
};

bool Assert(bool cond, Test_Info *info, const char *msg);

/// DEFINITION
/// @brief The container for a test itself, this is used only by the test
/// environment to keep track of it.
typedef struct
{
    Test_Procedure proc;
    Test_Info info;
    const char *name;
    int type;
} Test;

Test Create_Test(Test_Procedure proc, const char *name, int type);

//-------------------------------------------------------------------------------//
// test environment
//-------------------------------------------------------------------------------//

/// DEFINITION
/// @brief The environment that holds all of the tests and tracks their
/// status as they run, eventually reporting the results at the end.
///
/// This can be used to run a large number of tests at once and keep track
/// them in groups or as one large environment.
struct Test_Environment
{
    Test *tests;
    size_t count;
    size_t capacity;
    int completed;
    int successes;
    int failures;
};

Test_Environment *Init_Test_Environment();
void Load_Test(Test_Environment *env, Test test);
void Run_Battery(Test_Environment *env);
void Free_Test_Environment(Test_Environment *env);

//-------------------------------------------------------------------------------//
// test runners
//-------------------------------------------------------------------------------//

void Run_Test(Test *test);

#endif
