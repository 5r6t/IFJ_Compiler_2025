#include <stdio.h>
#include <string.h>
#include "../../include/symtable.h"

typedef enum direction { left, right, none } direction_t;

void fillBst(SymTableNode **tree, FILE *keywords);
void bst_print_tree(SymTableNode *tree);
void bst_print_subtree(SymTableNode *tree, char *prefix, direction_t from);
void bst_print_node(SymTableNode *node);
char *make_prefix(char *prefix, const char *suffix);

int main(int argc, char *argv[]){
    (void)argc;
	
    FILE *keywords = fopen(argv[1], "r");
    SymTableNode *tree = NULL;
    bst_init(tree);

    fillBst(&tree, keywords); // fill the tree with keywords
    bst_print_tree(tree);
    fclose(keywords);

    // test for search
    if(bst_search(tree, "while")){
        printf("\nfound\n");
    }else{
        printf("\nNOT found\n");
    }
    return 0;
}

void fillBst(SymTableNode **tree, FILE *keywords){
    printf("fillBst{\n");
    char c, buff[MAX_ID_LENGTH] = {0};
    unsigned idx = 0;
    while((c = fgetc(keywords)) != EOF){
        if(c == '\n'){
            *tree = bst_insert(*tree, buff);
            for(unsigned i=0;buff[i] != '\0';i++){
                buff[i] = 0;
            }
            idx = 0;
            continue;
        }
        buff[idx++] = c;
    }
    printf("}");
}

//------------------------- Functions for printing the tree -------------------------

const char *subtree_prefix = "  |";
const char *space_prefix = "   ";

char *make_prefix(char *prefix, const char *suffix) {
  char *result = (char *)malloc(strlen(prefix) + strlen(suffix) + 1);
  strcpy(result, prefix);
  result = strcat(result, suffix);
  return result;
}

void bst_print_subtree(SymTableNode *tree, char *prefix, direction_t from) {
  if (tree != NULL) {
    char *current_subtree_prefix = make_prefix(prefix, subtree_prefix);
    char *current_space_prefix = make_prefix(prefix, space_prefix);

    if (from == left) {
      printf("%s\n", current_subtree_prefix);
    }

    bst_print_subtree(
        tree->right,
        from == left ? current_subtree_prefix : current_space_prefix, right);

    printf("%s  +-", prefix);
    bst_print_node(tree);
    printf("\n");

    bst_print_subtree(
        tree->left,
        from == right ? current_subtree_prefix : current_space_prefix, left);

    if (from == right) {
      printf("%s\n", current_subtree_prefix);
    }

    free(current_space_prefix);
    free(current_subtree_prefix);
  }
}

void bst_print_tree(SymTableNode *tree) {
  printf("\n");
  if (tree != NULL) {
    bst_print_subtree(tree, "", none);
  } else {
    printf("Tree is empty\n");
  }
  printf("\n");
}

void bst_print_node(SymTableNode *node)
{
  printf("[");
  for(unsigned i=0;node->id[i] != '\0';i++){
    printf("%c", node->id[i]);
  }
  printf("]");
}

//-----------------------------------------------------------------------------------

