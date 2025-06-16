#include "lexer.h"
#include "parser.h"
#include "errors.h"
#include <stdio.h>

/* TODO & CONSIDERATIONS
 * 
 * [x] create a union in the error type to hold special error data
 * [x] rework the ast structure to use node ids in a contiguous array
 * write a test framework instead of this other garbage
 * store x and y componenets in token struct
 * 
 * 
 * 
 */

int main() {
    // printf("%s", SOURCE);

    // Test_Error_Printing();
    // Test_Error_Collection();
    // Test_Append_Data_Errors();
    Test_Lexer();


    return 0;
}