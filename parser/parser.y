%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "syntax_tree.h"

#include "lexer.h"

// external flex functions
extern int yylineno;
extern FILE* yyin;
extern int yylex();
void yyerror(const char* s);

// extern vars from lexer
extern int lines;
extern char* yytext;

// syntax tree
struct syn_tree* syntax_tree;
%}

%union {
    struct tree_node* pnode;
    char* n;
    double d;
}

/* ************************************
 * Token definitions
 * ************************************
 */
%token <pnode> tok_error tok_add tok_sub tok_mul tok_div tok_lt tok_lte
%token <pnode> tok_gt tok_gte tok_eq tok_neq tok_assign tok_semicolon
%token <pnode> tok_comma tok_l_parenthese tok_r_parenthese tok_l_bracket
%token <pnode> tok_r_bracket tok_l_brace tok_r_brace tok_else tok_if tok_int tok_return
%token <pnode> tok_void tok_while tok_identifier tok_number tok_array
%token <pnode> tok_letter tok_eol tok_comment tok_blank

/* ************************************
 * Type definitions
 * ************************************
 */
%type <pnode> declaration-list declaration var-declaration type-specifier
%type <pnode> fun-declaration params param-list param compound-stmt local-declarations
%type <pnode> statement-list statement expression-stmt selection-stmt iteration-stmt
%type <pnode> return-stmt expression var simple-expression relop additive-expression 
%type <pnode> addop term mulop factor call args arg-list

/* ************************************
 * Starting symbol
 * ************************************
 */
%start program

/* ************************************
 * Bison rules start here
 * ************************************
 */
%%
program: declaration-list
    {
        struct tree_node* tmp = newSynTreeNode("program");
        synTreeNodeAddChild(tmp, $1);
        syntax_tree->root = tmp;
    };

declaration-list: declaration-list declaration
    {
        $$ = newSynTreeNode("declaration-list");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | declaration
    {
        $$ = newSynTreeNode("declaration-list");
        synTreeNodeAddChild($$, $1);
    };

declaration: var-declaration
    {
        $$ = newSynTreeNode("declaration");
        synTreeNodeAddChild($$, $1);
    }
    | fun-declaration
    {
        $$ = newSynTreeNode("declaration");
        synTreeNodeAddChild($$, $1);
    };

var-declaration: type-specifier tok_identifier tok_semicolon
    {
        $$ = newSynTreeNode("var-declaration");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | type-specifier tok_identifier tok_l_bracket tok_number tok_r_bracket tok_semicolon
    {
        $$ = newSynTreeNode("var-declaration");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
        synTreeNodeAddChild($$, $5);
        synTreeNodeAddChild($$, $6);
    };

type-specifier: tok_int
    {
        $$ = newSynTreeNode("type-specifier");
        synTreeNodeAddChild($$, $1);
    }
    | tok_void
    {
        $$ = newSynTreeNode("type-specifier");
        synTreeNodeAddChild($$, $1);
    };

fun-declaration: type-specifier tok_identifier tok_l_parenthese params tok_r_parenthese compound-stmt
    {
        $$ = newSynTreeNode("fun-declaration");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
        synTreeNodeAddChild($$, $5);
        synTreeNodeAddChild($$, $6);
    };

params: param-list
    {
        $$ = newSynTreeNode("params");
        synTreeNodeAddChild($$, $1);
    }
    | tok_void
    {
        $$ = newSynTreeNode("params");
        synTreeNodeAddChild($$, $1);
    };

param-list: param-list tok_comma param
    {
        $$ = newSynTreeNode("param-list");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | param
    {
        $$ = newSynTreeNode("param-list");
        synTreeNodeAddChild($$, $1);
    };

param: type-specifier tok_identifier
    {
        $$ = newSynTreeNode("param");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | type-specifier tok_identifier tok_array
    {
        $$ = newSynTreeNode("param");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    };

compound-stmt: tok_l_brace local-declarations statement-list tok_r_brace
    {
        $$ = newSynTreeNode("compound-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
    };

local-declarations: local-declarations var-declaration
    {
        $$ = newSynTreeNode("local-declarations");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | /* Empty */
    {
        $$ = newSynTreeNode("local-declarations");
        // empty here
    };

statement-list: statement-list statement
    {
        $$ = newSynTreeNode("statement-list");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | /* Empty */
    {
        $$ = newSynTreeNode("statement-list");
        // empty here
    };

statement: expression-stmt
    {
        $$ = newSynTreeNode("statement");
        synTreeNodeAddChild($$, $1);
    }
    | compound-stmt
    {
        $$ = newSynTreeNode("statement");
        synTreeNodeAddChild($$, $1);
    }
    | selection-stmt
    {
        $$ = newSynTreeNode("statement");
        synTreeNodeAddChild($$, $1);
    }
    | iteration-stmt
    {
        $$ = newSynTreeNode("statement");
        synTreeNodeAddChild($$, $1);
    }
    | return-stmt
    {
        $$ = newSynTreeNode("statement");
        synTreeNodeAddChild($$, $1);
    };

expression-stmt: expression tok_semicolon
    {
        $$ = newSynTreeNode("expression-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | tok_semicolon
    {
        $$ = newSynTreeNode("expression-stmt");
        synTreeNodeAddChild($$, $1);
    };

selection-stmt: tok_if tok_l_parenthese expression tok_r_parenthese statement
    {
        $$ = newSynTreeNode("selection-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
        synTreeNodeAddChild($$, $5);
    }
    | tok_if tok_l_parenthese expression tok_r_parenthese statement tok_else statement
    {
        $$ = newSynTreeNode("selection-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
        synTreeNodeAddChild($$, $5);
        synTreeNodeAddChild($$, $6);
        synTreeNodeAddChild($$, $7);
    };

iteration-stmt: tok_while tok_l_parenthese expression tok_r_parenthese statement
    {
        $$ = newSynTreeNode("iteration-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
        synTreeNodeAddChild($$, $5);
    };

return-stmt: tok_return tok_semicolon
    {
        $$ = newSynTreeNode("return-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
    }
    | tok_return expression tok_semicolon
    {
        $$ = newSynTreeNode("return-stmt");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    };

expression: var tok_assign expression
    {
        $$ = newSynTreeNode("expression");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | simple-expression
    {
        $$ = newSynTreeNode("expression");
        synTreeNodeAddChild($$, $1);
    };

var: tok_identifier
    {
        $$ = newSynTreeNode("var");
        synTreeNodeAddChild($$, $1);
    }
    | tok_identifier tok_l_bracket expression tok_r_bracket
    {
        $$ = newSynTreeNode("var");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
    };

simple-expression: additive-expression relop additive-expression
    {
        $$ = newSynTreeNode("simple-expression");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | additive-expression
    {
        $$ = newSynTreeNode("simple-expression");
        synTreeNodeAddChild($$, $1);
    };

relop: tok_lte
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_lt
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_gt
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_gte
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_eq
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_neq
    {
        $$ = newSynTreeNode("relop");
        synTreeNodeAddChild($$, $1);
    };

additive-expression: additive-expression addop term
    {
        $$ = newSynTreeNode("additive-expression");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | term
    {
        $$ = newSynTreeNode("additive-expression");
        synTreeNodeAddChild($$, $1);
    };

addop: tok_add
    {
        $$ = newSynTreeNode("addop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_sub
    {
        $$ = newSynTreeNode("addop");
        synTreeNodeAddChild($$, $1);
    };

term: term mulop factor
    {
        $$ = newSynTreeNode("term");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | factor
    {
        $$ = newSynTreeNode("term");
        synTreeNodeAddChild($$, $1);
    };

mulop: tok_mul
    {
        $$ = newSynTreeNode("mulop");
        synTreeNodeAddChild($$, $1);
    }
    | tok_div
    {
        $$ = newSynTreeNode("mulop");
        synTreeNodeAddChild($$, $1);
    };

factor: tok_l_parenthese expression tok_r_parenthese
    {
        $$ = newSynTreeNode("factor");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | var
    {
        $$ = newSynTreeNode("factor");
        synTreeNodeAddChild($$, $1);
    }
    | call
    {
        $$ = newSynTreeNode("factor");
        synTreeNodeAddChild($$, $1);
    }
    | tok_number
    {
        $$ = newSynTreeNode("factor");
        synTreeNodeAddChild($$, $1);
    };

call: tok_identifier tok_l_parenthese args tok_r_parenthese
    {
        $$ = newSynTreeNode("call");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
        synTreeNodeAddChild($$, $4);
    };

args: arg-list
    {
        $$ = newSynTreeNode("args");
        synTreeNodeAddChild($$, $1);
    }
    | /* empty */
    {
        $$ = newSynTreeNode("args");
        // empty here
    };

arg-list: arg-list tok_comma expression
    {
        $$ = newSynTreeNode("arg-list");
        synTreeNodeAddChild($$, $1);
        synTreeNodeAddChild($$, $2);
        synTreeNodeAddChild($$, $3);
    }
    | expression
    {
        $$ = newSynTreeNode("arg-list");
        synTreeNodeAddChild($$, $1);
    };
%%

/* ************************************
 * C functions 
 * ************************************
 */
void yyerror(const char* s) {
    fprintf(stderr, "%s:%d syntax error for %s\n", s, yylineno, yytext);
}

// test function for parser
void parser(const char* input_file_name, const char* output_file_name) {
    // initialize syntax tree
    syntax_tree = newSynTree();

    // initialize input and output path
    char input_path[256] = "./testcase/";
    char output_path[256] = "./testout/parser/";

    strcat(input_path, input_file_name);
    strcat(output_path, output_file_name);

    // open file for analysis
    if(!(yyin = fopen(input_path, "r"))) {
        PANIC("Open file %s failed.\n", input_path);
    }

    //yyrestart(yyin);

    DEBUG_PRINT("Parser start for %s\n", input_file_name);

    FILE* fp = fopen(output_path, "w+");

    if(!fp) {
        PANIC("Open output file %s failed.\n", output_path);
    }

    yylineno = 1;   // reset line number

    yyparse();

    DEBUG_PRINT("The syntax tree is printed to %s\n", output_path);

    printSynTree(fp, syntax_tree);

    deleteSynTree(syntax_tree);

    syntax_tree = NULL;

    fclose(fp);

    DEBUG_PRINT("Syntax analyse for %s is done.\n", output_path);

    return;
}

// parser test main
int parser_main(int argc, char** argv) {
    char filenames[50][256];
    char output_file_name[256];
    const char* suffix = ".syntax_tree";

    int fn = getAllTestCases(filenames);

    int cnt;
    char* tmp;
    
    for(int i = 0; i < fn; ++ i) {
        cnt = 0;
        tmp = filenames[i];

        while(*(tmp + cnt) != '.') {
            cnt ++;
        }

        strncpy(output_file_name, tmp, cnt);
        strcat(output_file_name, suffix);

        parser(filenames[i], output_file_name);
        memset(output_file_name, 0, strlen(output_file_name));
    }

    return 0;
}