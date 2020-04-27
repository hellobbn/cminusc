#include "syntax_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** newSynTreeNode_noname():
 *  create a syntax tree node without a name.
 *
 *  @return a syntax tree node
 */
struct tree_node *newSynTreeNode_noname(void) {
  struct tree_node *tmp = malloc(sizeof(struct tree_node));

  tmp->child_num = 0;
  tmp->name[0] = '\0';

  return tmp;
}

/** newSynTreeNode()
 *  create a syntax tree node with a name.
 *
 *  @param name the name of the tree node
 *  @return a syntax tree node
 */
struct tree_node *newSynTreeNode(const char *name) {
  struct tree_node *tmp = malloc(sizeof(struct tree_node));

  if (name) {
    strcpy(tmp->name, name);
  } else {
    tmp->name[0] = '\0';
  }

  tmp->child_num = 0;
  return tmp;
}

/** newSynTreeNode_fromnum():
 *  create a syntax tree node from a number.
 *
 *  @param num the number
 *  @return a syntax tree node
 */
struct tree_node *newSynTreeNode_fromnum(const int num) {
  struct tree_node *tmp = malloc(sizeof(struct tree_node));

  sprintf(tmp->name, "%d", num);

  tmp->child_num = 0;

  return tmp;
}

/** synTreeNodeAddChild():
 *  add a syntax tree node to the parent node.
 *
 *  @param parent the parent node
 *  @param child the child node
 */
void synTreeNodeAddChild(struct tree_node *parent, struct tree_node *child) {
  if (!parent || !child) {
    return;
  }
  parent->children[parent->child_num++] = child;
}

/** synTreeNodeDelChild_noRecur():
 *  delete syntex tree node without recur
 *
 *  @param the node to delete
 */
void synTreeNodeDelChild_noRecur(struct tree_node *node) {
  if (!node) {
    return;
  }

  free(node);
}

/** synTreeNodeDelChild():
 *  delete syntex tree node, and all children
 *
 *  @param the node to delete
 */
void synTreeNodeDelChild(struct tree_node *node) {
  if (!node) {
    return;
  }

  int i;
  for (i = 0; i < node->child_num; i++) {
    synTreeNodeDelChild(node->children[i]);
  }

  free(node);
}

/** newSynTree():
 *  create a new syntex tree
 */
struct syn_tree *newSynTree(void) {
  return malloc(sizeof(struct syn_tree));
}

/** deleteSynTree():
 *  free a syntax tree
 *
 *  @param tree the syntax tree
 */
void deleteSynTree(struct syn_tree *tree) {
  if (!tree) {
    return;
  }

  if (tree->root) {
    synTreeNodeDelChild(tree->root);
  }

  free(tree);
}

/** printSyntreeNode():
 *  static function, print a syntax tree node
 *
 *  @param string the output string
 *  @param node the node to print
 *  @param level the level of the node
 */
static void printSyntreeNode(FILE *string, struct tree_node *node, int level) {
  if (!node) {
    return;
  }

  // print myself
  int i;
  for (i = 0; i < level; ++i) {
    fprintf(string, "|  ");
  }

  fprintf(string, ">--%s %s\n", (node->child_num ? "+" : "*"), node->name);

  for (i = 0; i < node->child_num; ++i) {
    printSyntreeNode(string, node->children[i], level + 1);
  }
}

/** printSynTree:
 *  prints out a syntax tree
 *
 *  @param string the output string
 *  @param tree the tree to print
 */
void printSynTree(FILE *string, struct syn_tree *tree) {
  if (!string) {
    return;
  }

  printSyntreeNode(string, tree->root, 0);
}