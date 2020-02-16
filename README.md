# Cminusc

Compiler for C-. 

This is a re-mastered project for Compiler Principle @ USTC 2019 Fall.

The project is re-organised and re-written by bbn.

## A. roadmap

- [x] Lexer (flex)
- [x] Parser (bison)
- [x] llvm code gen
- [ ] System V calling ABI
- [ ] testcase
- [ ] built-in functions
- [ ] compile to exec

## B. build

use command `make` to generate a test binary for the lexer, the syntax tree, the parser, it will also generate the actual C- compiler, located in `build/cminus`

## C. Credit

Some source code are referenced directly from TA's source code.

See CREDIT.md
