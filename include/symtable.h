//////////////////////////////////////////////
// filename: symtable.h                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ID_LENGTH 100

typedef struct STN {
    int type;
    int height;
    char id[MAX_ID_LENGTH];
    struct STN *left;
    struct STN *right;
} SymTableNode;

void bst_init(SymTableNode *tree);
void bstDelete(SymTableNode *tree);

bool bst_search(SymTableNode *tree, char id[]);

int bst_getHeight(SymTableNode *tree);
int max(int a, int b);
int bst_getBalance(SymTableNode *tree);
SymTableNode *bst_createNode(char id[]);
SymTableNode *bst_rightRotate(SymTableNode *tree);
SymTableNode *bst_leftRotate(SymTableNode *tree);
SymTableNode *bst_insert(SymTableNode *tree, char id[]);