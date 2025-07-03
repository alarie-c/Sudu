#ifndef FRONTEND_H
#define FRONTEND_H

#define INIT_TOKEN_CAPACITY 1024
#define INIT_PROGRAM_CAPACITY 64

#include "frontend/ast.h"
#include "util/tests.h"

/// @brief Top-level frontend function that turns the source code into an abstract syntax tree.
/// @param src the source code loaded into memory.
/// @param path the path to the file for this source code.
/// @param diagnostics a dynamic array of `Error` instances for `Parse` to write to.
/// @return AST struct containing the contiguous array for the nodes along with the root node itself.
Abstract_Syntax_Tree Parse(const char *src, const char *path, List *diagnostics);

//===============================================================================//
// TESTS
//===============================================================================//

void Test_Lexer(Test_Info *info);
void Test_Parser(Test_Info *info); 

#endif // FRONTEND_H
