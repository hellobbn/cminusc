%option noyywrap
%option yylineno

%{
/* ------------------------------------
 * includes and definitions
 * ------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "helper.h"
//#include "syntax_tree.h"

int file_count = 0;
int lines;
int pos_start;
int pos_end;
int prev_line;
%}

commentPattern "/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"
identifierPattern [a-zA-Z][a-zA-Z]*

%%

{commentPattern} {
    return tok_comment;
}

"[]" {
    return tok_array;
}

"+" {
    return tok_add;
}

"-" {
    return tok_sub;
}

"*" {
    return tok_mul;
}

"/" {
    return tok_div;
}

"<" {
    return tok_lt;
}

"<=" {
    return tok_lte;
}

">" {
    return tok_gt;
}

">=" {
    return tok_gte;
}

"==" {
    return tok_eq;
}

"!=" {
    return tok_neq;
}

"=" {
    return tok_assign;
}

";" {
    return tok_semicolon;
}

"," {
    return tok_comma;
}

"(" {
    return tok_l_parenthese;
}

")" {
    return tok_r_parenthese;
}

"[" {
    return tok_l_bracket;
}

"]" {
    return tok_r_bracket;
}

"{" {
    return tok_l_brace;
}

"}" {
    return tok_r_brace;
}

"else" {
    return tok_else;
}

"if" {
    return tok_if;
}

"int" {
    return tok_int;
}

"return" {
    return tok_return;
}

"void" {
    return tok_void;
}

"while" {
    return tok_while;
}

{identifierPattern} {
    return tok_identifier;
}

[0-9][0-9]* {
    return tok_number;
}

\n {
    return tok_eol;
}

[" "\t] {
    return tok_blank;
}

. {
    return error;
}

%%

/** analyzer():
 *  test function: tests the lex analyzer
 */
void analyzer(char* input_file_name, char* output_file_name) {
    prev_line = yylineno;

    char input_path[256] = "./testcase/";
    char output_path[256] = "./testout/lexer/";

    strcat(input_path, input_file_name);
    strcat(output_path, output_file_name);

    if(!(yyin = fopen(input_path, "r"))) {
        PANIC("No input file, expected %s", input_path);
    }

    DEBUG_PRINT("Read from %s\n", input_file_name);
    
    FILE* fp = fopen(output_path, "w+");

    int token;

    // initialize scanner pointer
    pos_start = 1;
    pos_end = 1;
    while((token = yylex())) {
        lines += yylineno - prev_line;
        int i;
        for(i = 0; i < yyleng; ++ i) {
            if(yytext[i] != '\n') {
                pos_end += 1;
            } else {
                pos_start += 1;
                pos_end = 1;
            }
        }

        switch(token) {
            case tok_eol:
                break;
            case error:
                fprintf(fp, "[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
                break;
            case tok_comment:
            case tok_blank:
                break;
            case tok_number:
                fprintf(fp, "%d\t%s\t%d\t%d\t%d\n",atoi(yytext), token_to_str(token), lines, pos_start, pos_end);
                break;
            default:
                fprintf(fp, "%s\t%s\t%d\t%d\t%d\n",yytext, token_to_str(token), lines, pos_start, pos_end);
        }

        prev_line = yylineno;
        pos_start = pos_end;
    }
    fclose(fp);
    DEBUG_PRINT("analyze completed\n");
}

/** lex_main():
 *  test function for lexer
 */
 int lex_main(int argc, char** argv) {
     char filenames[50][256];
     char output_file_name[256];
     char suffix[] = ".tokens";

     file_count = getAllTestCases(filenames);

     int cnt;
     char* str_tmp;
     prev_line = 0;

     for(int i = 0; i < file_count; ++ i) {
         // process input
         cnt = 0;
         str_tmp = filenames[i];

         while(*(str_tmp + cnt) != '.') {
             cnt ++;
         }

         strncpy(output_file_name, str_tmp, cnt);
         strcat(output_file_name, suffix);

         lines = 1;
         analyzer(filenames[i], output_file_name);
         memset(output_file_name, 0, strlen(output_file_name));
     }

     return 0;
 }