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

#include "parser.tab.h"
#include "helper.h"
#include "syntax_tree.h"
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
#ifdef LEX_TEST
    return tok_comment;
#endif
}

"[]" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("[]");
#endif
    return tok_array;
}

"+" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("+");
#endif
    return tok_add;
}

"-" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("-");
#endif
    return tok_sub;
}

"*" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("*");
#endif
    return tok_mul;
}

"/" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("/");
#endif
    return tok_div;
}

"<" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("<");
#endif
    return tok_lt;
}

"<=" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("<=");
#endif
    return tok_lte;
}

">" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(">");
#endif
    return tok_gt;
}

">=" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(">=");
#endif
    return tok_gte;
}

"==" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("==");
#endif
    return tok_eq;
}

"!=" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("!=");
#endif
    return tok_neq;
}

"=" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("=");
#endif
    return tok_assign;
}

";" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(";");
#endif
    return tok_semicolon;
}

"," {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(",");
#endif
    return tok_comma;
}

"(" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("(");
#endif
    return tok_l_parenthese;
}

")" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(")");
#endif
    return tok_r_parenthese;
}

"[" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("[");
#endif
    return tok_l_bracket;
}

"]" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("]");
#endif
    return tok_r_bracket;
}

"{" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("{");
#endif
    return tok_l_brace;
}

"}" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("}");
#endif
    return tok_r_brace;
}

"else" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("else");
#endif
    return tok_else;
}

"if" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("if");
#endif
    return tok_if;
}

"int" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("int");
#endif
    return tok_int;
}

"return" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("return");
#endif
    return tok_return;
}

"void" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("void");
#endif
    return tok_void;
}

"while" {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode("while");
#endif
    return tok_while;
}

{identifierPattern} {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(yytext);
#endif
    return tok_identifier;
}

[0-9][0-9]* {
#ifndef LEX_TEST
    yylval.pnode = newSynTreeNode(yytext);
#endif
    return tok_number;
}

\n {
#ifdef LEX_TEST
    return tok_eol;
#endif
}

[" "\t] {
#ifdef LEX_TEST
    return tok_blank;
#endif
}

. {
    return tok_error;
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
            case tok_error:
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