// header for syntax tree
#ifndef SYNTAX_TREE
#define SYNTAX_TREE

// syntax tree C structure
#include <stdio.h>

// syntax tree node
struct tree_node {
  struct tree_node *parent;
  struct tree_node *children[1000];
  int child_num;

  char name[30];
};
typedef struct tree_node tree_node;

struct tree_node *newSynTreeNode_noname();
struct tree_node *newSynTreeNode(const char *name);
struct tree_node *newSynTreeNode_fromnum(const int num);
void synTreeNodeAddChild(struct tree_node *parent, struct tree_node *child);
void synTreeNodeDelChild_noRecur(struct tree_node *node);
void synTreeNodeDelChild(struct tree_node *node);

struct syn_tree {
  struct tree_node *root;
};
typedef struct syn_tree syn_tree;

struct syn_tree *newSynTree(void);
void deleteSynTree(struct syn_tree *tree);
void printSynTree(FILE *string, struct syn_tree *tree);

#endif /* SYNTAX_TREE */