# Sudu Language Implemention

Sudu closely translates to "speed" in Mandarin Chinese.
To be implemented as a bytecode interpreter on a virtual machine...
Statically and explicitly typed, with some flexibility.

## Progress Tracker

### General
- [x] Rework to use single-function interface with private internal state structs
- [x] Rework into one `frontend.h` interface with `parser.c` and `lexer.c` implementations
- [x] Update the build script to allow for tests/compilation and for aborting on compilation errors
- [ ] Implement binary postfixup algorithm for the binary expression parser
- [x] Create a generalized List structure for dynamic arrays
- [x] Implement `List<Error>` for error collection
- [x] Implement `List<Ast_Node>` for the parser

### Systems
- [ ] Parser
    - [x] Binary Infix Expressions
    - [ ] Unary Prefix Expressions
    - [ ] Function Declarations
    - [x] Function Calls
    - [x] Variable Declaratiosn
    - [x] Variable Assignment
- [ ] Sematic Analyzer
    - [ ] Type Checker
    - [ ] Name Resolution
    - [ ] Scope/Context Tracking
- [ ] Bytecode Compiler
    - [ ] Register Allocator
    - [ ] Basic Expressions like '5 + 10 + 30'


