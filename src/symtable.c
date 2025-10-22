//////////////////////////////////////////////
// filename: symtable.c                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/symtable.h"

/*NOTES:
  - need to add stack for scopes
  - need to add types for variables
*/

/**
 * @brief Initializes the binary search tree.
 * 
 * @param tree pointer to the root of the binary search tree
 */
void bst_init(SymTableNode *tree){
    tree = NULL;
    (void)tree;
}

/**
 * @brief Searches for an identifier in the binary search tree
 * 
 * @param tree pointer to the root of the binary search tree
 * @param id searched identifier
 * @return true if found
 * @return false if not found
 */
bool bst_search(SymTableNode *tree, char id[]){
    while(tree != NULL){
        int compare = strcmp(tree->id, id);

        if(compare == 0){
            return true;
        }else if(compare > 0){
            tree = tree->left;
        }else{
            tree = tree->right;
        }
    }
    return false;
}

/**
 * @brief get the height of the tree
 * 
 * @param tree pointer to the root of the binary search tree
 * @return height of the tree
 */
int bst_getHeight(SymTableNode *tree){
    if(tree == NULL){
        return 0;
    }
    return tree->height;
}

/**
 * @brief helper function to get the maximum of two integers
 * 
 * @param a first input parameter
 * @param b second input parameter
 * @return maximum of a and b
 */
int max(int a, int b){
    return (a > b) ? a : b;
}

/**
 * @brief gets the balance factor of the tree
 * 
 * @param tree pointer to the root of the binary search tree
 * @return balance factor
 */
int bst_getBalance(SymTableNode *tree){
    if(tree == NULL){
        return 0;
    }
    return (bst_getHeight(tree->left) - bst_getHeight(tree->right));
}

/**
 * @brief creates a new node for the binary search tree
 * 
 * @param id identifier for the new node
 * @return pointer to the newly created node
 */
SymTableNode *bst_createNode(char id[]){
    SymTableNode *new = (SymTableNode *)malloc(sizeof(SymTableNode)); // creating new node
    if(new == NULL){
        fprintf(stderr, "Error with memory allocation");
        return NULL;
    }

    unsigned j;
    for(j=0;id[j] != '\0';j++){
        new->id[j] = id[j];
    }
    new->id[j] = '\0';

    new->left = NULL;
    new->right = NULL;
    new->type = 0;
    new->height = 1;

    return new;
}

/**
 * @brief rotates the tree to the right
 * 
 * @param tree pointer to the root of the binary search tree
 * @return new root after rotation
 */
SymTableNode *bst_rightRotate(SymTableNode *tree){
    SymTableNode *x = tree->left;
    SymTableNode *T2 = x->right;

    x->right = tree;
    tree->left = T2;

    tree->height = max(bst_getHeight(tree->left), bst_getHeight(tree->right)) + 1;
    x->height = max(bst_getHeight(x->left), bst_getHeight(x->right)) + 1;

    return x;
}

/**
 * @brief rotates the tree to the left
 * 
 * @param tree pointer to the root of the binary search tree
 * @return new root after rotation
 */
SymTableNode *bst_leftRotate(SymTableNode *tree){
    SymTableNode *y = tree->right;
    SymTableNode *T2 = y->left;

    y->left = tree;
    tree->right = T2;

    tree->height = max(bst_getHeight(tree->left), bst_getHeight(tree->right)) + 1;
    y->height = max(bst_getHeight(y->left), bst_getHeight(y->right)) + 1;

    return y;
}

/**
 * @brief inserts a new identifier into the binary search tree
 * 
 * @param tree pointer to the root of the binary search tree
 * @param id identifier to be inserted
 * @return pointer to the root of the modified binary search tree
 */
SymTableNode *bst_insert(SymTableNode *tree, char id[]){
    if(tree == NULL){
        return bst_createNode(id);
    }

    if(strcmp(id, tree->id) < 0){
        tree->left = bst_insert(tree->left, id);
    }else if(strcmp(id, tree->id) > 0){
        tree->right = bst_insert(tree->right, id);
    }else{
        return tree;
    }

    tree->height = max(bst_getHeight(tree->left), bst_getHeight(tree->right)) + 1;

    int balance = bst_getBalance(tree);

    if(balance > 1 && strcmp(id, tree->left->id) < 0){ // LL
        return bst_rightRotate(tree);
    }

    if(balance < -1 && strcmp(id, tree->right->id) > 0){ // PP
        return bst_leftRotate(tree);
    }

    if(balance > 1 && strcmp(id, tree->left->id) > 0){ // LP
        tree->left = bst_leftRotate(tree->left);
        return bst_rightRotate(tree);
    }

    if(balance < -1 && strcmp(id, tree->right->id) < 0){ // PL
        tree->right = bst_rightRotate(tree->right);
        return bst_leftRotate(tree);
    }

    return tree;
}