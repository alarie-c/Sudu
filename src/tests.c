#include "tests.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Test Create_Test(Test_Procedure proc, const char *name, int type)
{
    Test_Info info = (Test_Info) {.success = 0, .status = 0, .message = "<no message>"};
    return (Test) {.info = info, .name = name, .proc = proc, .type = type};
}   

//-------------------------------------------------------------------------------//
// test environment
//-------------------------------------------------------------------------------//

Test_Environment *Init_Test_Environment()
{
    Test_Environment *env = malloc(sizeof(Test_Environment));
    if (!env) return NULL;
    memset(env, 0, sizeof(Test_Environment));

    // allocate the test array
    Test *tests = malloc(INIT_TEST_CAPACITY * sizeof(Test));
    if (!tests)
    {
        free(env);
        return NULL;
    }

    env->tests = tests;
    env->capacity = INIT_TEST_CAPACITY;
    return env;
}

void Load_Test(Test_Environment *env, Test test)
{
    if (!env) return;

    if (env->count >= env->capacity)
    {
        size_t new_cap = env->capacity * 2;
        Test *new_tests = realloc(env->tests, new_cap * sizeof(Test));
        if (!new_tests) return; // (TODO) silent error
        env->tests = new_tests;
        env->capacity = new_cap;
    }

    env->tests[env->count] = test;
    env->count++;
}

void Run_Battery(Test_Environment *env)
{
    printf("\n%s[TESTS]%s BEGINNING TEST BATTERY.\n", T_TERM_CYANB, T_TERM_RESET);

    for (size_t i = 0; i < env->count; i++)
    {
        Test *test = &env->tests[i];
        if (!test) break;

        Run_Test(test);
        if (test->info.status)   env->completed++;
        if (test->info.success)  env->successes++;
        else                     env->failures++;
    }

    printf("\n%s[TESTS]%s TEST BATTERY FINISHED.\n", T_TERM_CYANB, T_TERM_RESET);
    printf("> Tests Completed: %i\n", env->completed);
    printf("> Tests Failed   : %i\n", env->failures);
    printf("> Tests Succeeded: %i\n", env->successes);
    if (env->failures == 0)
    {
        printf("%sALL TESTS PASSED\n%s", T_TERM_GREENB, T_TERM_RESET);
    } 
    else
    {
        printf("%sFAILURES FOUND, TESTS UNSUCCESSFUL\n%s", T_TERM_REDB, T_TERM_RESET);
    }
}

void Free_Test_Environment(Test_Environment *env)
{
    if (!env) return;

    free(env->tests);
    free(env);
}

//-------------------------------------------------------------------------------//
// runners
//-------------------------------------------------------------------------------//

void Run_Test(Test *test)
{
    printf("\n%s[TESTS]%s %s'%s'%s.\n\n",
        T_TERM_CYANB,
        T_TERM_RESET,
        T_TERM_YELLOWI,
        test->name,
        T_TERM_RESET    
    );
    
    // run the test procedure
    test->proc(&test->info);

    if (!test->info.status)
    {
        printf("\n%s[TESTS]%s %s'%s'%s %sFAILED%s TO COMPELTE!\n  MSG: '%s'.\n\n",
            T_TERM_CYANB,
            T_TERM_RESET,
            T_TERM_YELLOWI,
            test->name,
            T_TERM_RESET,
            T_TERM_REDB,
            T_TERM_RESET,
            test->info.message
        );
        return;
    }

    if (test->type == TEST_TYPE_ASSERTION)
    {
        if (test->info.success)
        {
            printf("\n%s[TESTS]%s %s'%s'%s COMPLETE and %sPASSED%s.\n\n",
                T_TERM_CYANB,
                T_TERM_RESET,
                T_TERM_YELLOWI,
                test->name,
                T_TERM_RESET,
                T_TERM_GREENB,
                T_TERM_RESET
            );
        }
        else
        {
            printf("\n%s[TESTS]%s %s'%s'%s COMPLETE and %sFAILED%s.\n\n",
                T_TERM_CYANB,
                T_TERM_RESET,
                T_TERM_YELLOWI,
                test->name,
                T_TERM_RESET,
                T_TERM_REDB,
                T_TERM_RESET
            );
        }
        return;
    }

    if (test->type == TEST_TYPE_MANUAL)
    {
        printf("\n%s[TESTS]%s %s'%s'%s COMPLETED.\n\n",
            T_TERM_CYANB,
            T_TERM_RESET,
            T_TERM_YELLOWI,
            test->name,
            T_TERM_RESET    
        );
        return;
    }
}
    