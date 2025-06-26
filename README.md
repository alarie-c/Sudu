# Sudu Language Implemention

Sudu closely translates to "speed" in Mandarin Chinese.
To be implemented as a bytecode interpreter on a virtual machine...
Statically and explicitly typed, with some flexibility.

```
greeting = func(name: str)
    print("Hello, ${name}")
end

main = func()
    greeting("World")
end
```

## Progress Tracker

### General
- [ ] Update the build script to allow for tests/compilation and for aborting on compilation errors
- [ ] Implement binary postfixup algorithm for the binary expression parser

### Systems
- [ ] Parser
    - [x] Binary Infix Expressions
    - [ ] Unary Prefix Expressions
    - [ ] Function Declarations
    - [ ] Function Calls
    - [ ] Variable Declaratiosn
    - [ ] Variable Assignment
- [ ] Sematic Analyzer
    - [ ] Type Checker
    - [ ] Name Resolution
    - [ ] Scope/Context Tracking
- [ ] Bytecode Compiler
    - [ ] Register Allocator
    - [ ] Basic Expressions like '5 + 10 + 30'