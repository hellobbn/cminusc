// helper functions

#include "helper.h"
#include "lexer.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

/** getAllTestCases(char** name):
 *  get all file paths under `testcase` directory
 *
 *  @param name place to save all file names
 *  @return the number of test cases
 */
int getAllTestCases(char name[][256]) {
  DIR *working_dir;
  struct dirent *d;

  char cminus[] = "cminus";

  int i = 0;
  int files_count = 0;
  int status;

  working_dir = opendir("./testcase");

  if (working_dir == NULL) {
    PANIC("unable to find `testcase` folder\n");
  }

  while ((d = readdir(working_dir)) != NULL) {
    i = 0;

    char *file_name = d->d_name;
    while (file_name[i] != '.') {
      i = i + 1;
    }

    // check file extension
    status = strncmp(file_name + i + 1, cminus, 6);

    if (status == 0) {
      DEBUG_PRINT("find file %s\n", file_name);
      strcpy(name[files_count++], file_name);
    } else {
      // skip
    }
  }

  return files_count;
}

// pre-defined token string
char *tokens[] = {
    "ERROR",        "ADD",        "SUB",       "MUL",     "DIV",
    "LT",           "LTE",        "GT",        "GTE",     "EQ",
    "NEQ",          "ASSIGN",     "SEMICOLON", "COMMA",   "L_PARENTHESE",
    "R_PARENTHESE", "L_BRACKET",  "R_BRACKET", "L_BRACE", "R_BRACE",
    "ELSE",         "IF",         "INT",       "RETURN",  "VOID",
    "WHILE",        "IDENTIFIER", "NUMBER",    "ARRAY",   "LETTER",
    "EOL",          "COMMENT",    "BLANK"};

/** token_to_str():
 *  given a token number, converts it to string and return it.
 *
 *  @param token the token
 *  @return the string
 */
char *token_to_str(int token) {
  int i = -(token + 1);

  return tokens[i];
}