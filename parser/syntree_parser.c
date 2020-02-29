// The actual parser function for syntax tree
// tranform

/** syn_parser():
 *  for the compiler to use, the parser function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "syntax_tree.h"

#include "lexer.h"

// external functions
extern int yylineno;
extern FILE* yyin;
extern int yylex();
extern int yyparse();
void yyerror(const char* s);

// extern vars from lexer
extern int lines;
extern char* yytext;

// local global value
struct syn_tree* syntax_tree;

/** syn_parser():
 *  parser generater for the compiler
 *  
 *  @param input_file_name the file to open for parsing
 *  @return the syntax tree parsed by parser
 */
struct syn_tree* syn_parser(const char* input_file_name) {
    // initialize syntax tree to be returned
    syntax_tree = newSynTree();

    // get input file
    // char input_path[256] = "./testcase/";
    // strcat(input_path, input_file_name);

    // open file for analysis
    if(!(yyin = fopen(input_file_name, "r"))) {
        PANIC("Open file %s failed.\n", input_file_name);
    }

    // done, start parsing
    DEBUG_PRINT("Parser start for %s\n", input_file_name);

    yyparse();  // this operation constructs a syntax tree

    DEBUG_PRINT("Parser ended for %s\n", input_file_name);

    fclose(yyin);

    return syntax_tree;
}
