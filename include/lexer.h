#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

extern int fileno (FILE* __stream) __THROW __wur;

#ifndef YYTOKENTYPE
#define YYTOKENTYPE

typedef enum cminus_token_type {
    error = -1,
    tok_add = -2,
    tok_sub = -3,
    tok_mul = -4,
    tok_div = -5,
    tok_lt = -6,
    tok_lte = -7,
    tok_gt = -8,
    tok_gte = -9,
    tok_eq = -10,
    tok_neq = -11,
    tok_assign = -12,
    tok_semicolon = -13,
    tok_comma = -14,
    tok_l_parenthese = -15,
    tok_r_parenthese = -16,
    tok_l_bracket = -17,
    tok_r_bracket = -18,
    tok_l_brace = -19,
    tok_r_brace = -20,
    tok_else = -21,
    tok_if = -22,
    tok_int = -23,
    tok_return = -24,
    tok_void = -25,
    tok_while = -26,
    tok_identifier = -27,
    tok_number = -28,
    tok_array = -29,
    tok_letter = -30,
    tok_eol = -31,
    tok_comment = -32,
    tok_blank = -33
} Token;

#endif /* YYTOKENTYPE */

const char* strtoken(Token t);


int lex_main(int argc, char** argv);
#endif /* LEXER_H */