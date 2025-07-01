#include "frontend/parser.h"
#include "util/errors.h"
#include "util/common.h"
#include "util/tests.h"
#include <stdio.h>

void tests()
{
    Test_Environment *env = Init_Test_Environment();
    if (!env) return;

    Load_Test(env,
        Create_Test(
            Test_Parser,
            "Parser",
            TEST_TYPE_MANUAL
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