#include "syntax_tree.h"

int main(void) {
  struct syn_tree *tree = newSynTree();

  tree->root = newSynTreeNode("root");

  struct tree_node *node_a;
  struct tree_node *node_b;

  node_a = newSynTreeNode("a");
  node_b = node_a;
  synTreeNodeAddChild(tree->root, node_a);
  node_a = newSynTreeNode("b");
  synTreeNodeAddChild(tree->root, node_a);
  node_a = newSynTreeNode("c");
  synTreeNodeAddChild(tree->root, node_a);
  node_a = newSynTreeNode("aa");
  synTreeNodeAddChild(node_b, node_a);
  node_a = newSynTreeNode("ab");
  synTreeNodeAddChild(node_b, node_a);

  printSynTree(stdout, tree);

  return 0;
}