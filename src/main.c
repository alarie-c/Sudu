#include "lexer.h"
#include "parser.h"
#include "errors.h"
#include "common.h"
#include "tests.h"
#include <stdio.h>

void tests()
{
    Test_Environment *env = Init_Test_Environment();
    if (!env) return;

    Load_Test(env,
        Create_Test(
            Test_Binary_Expression,
            "Binary Expression Parser",
            TEST_TYPE_MANUAL
        )
    );

    Load_Test(env,
        Create_Test(
            Test_Grouping_Expression,
            "Grouping Expression Parser",
            TEST_TYPE_MANUAL
        )
    );

    Load_Test(env,
        Create_Test(
            Test_List,
            "List<int> Implementation",
            TEST_TYPE_MANUAL
        )
    );

    Load_Test(env,
        Create_Test(
            Test_Error_Collection,
            "Error Collection via List<Error>",
            TEST_TYPE_MANUAL
        )
    );

    Load_Test(env,
        Create_Test(
            Test_Append_Data_Errors,
            "Error Data Append",
            TEST_TYPE_ASSERTION
        )
    );

    Run_Battery(env);
    Free_Test_Environment(env);
}

int main()
{
    tests();
    return 0;
}