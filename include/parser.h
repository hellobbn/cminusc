#ifndef PARSER_H
#define PARSER_H

int parser_main(int argc, char **argv);

struct syn_tree *syn_parser(const char *input_file_name);

#endif /* PARSER_H */