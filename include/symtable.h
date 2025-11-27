//////////////////////////////////////////////
// filename: symtable.h                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

/**
 * @file symtable.h
 * @brief functions and structures for symbol table implementation
 *  
 */

#ifndef SYMTABLE
#define SYMTABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// TODO unite with lexer's MAX_ID_LENGTH
// #NEXT_MEETING
#define MAX_ID_LENGTH 100 // maximum length of an identifier
#define MAX_SCOPES 100 // maximum number of scopes in the stack

typedef struct STN {
    int type; // unite with lexer's token types #NEXT_MEETING
    int height;
    char id[MAX_ID_LENGTH];
    struct STN *left;
    struct STN *right;
} SymTableNode;

typedef struct {
	SymTableNode **array;
	int topIndex;
} Scopes;

typedef struct {
    Scopes *scopeStack;
} SemanticAnalyzer;

// ---- BST functions ----
void bst_init(SymTableNode *tree);
void bst_delete(SymTableNode *tree);

bool bst_search(SymTableNode *tree, char id[]);

int bst_getHeight(SymTableNode *tree);
int max(int a, int b);
int bst_getBalance(SymTableNode *tree);
SymTableNode *bst_createNode(char id[]);
SymTableNode *bst_rightRotate(SymTableNode *tree);
SymTableNode *bst_leftRotate(SymTableNode *tree);
SymTableNode *bst_insert(SymTableNode *tree, char id[]);


// -------- Scope stack functions ----
void scopeStack_init(Scopes *stack);
bool scopeStack_isEmpty(const Scopes *stack);
bool scopeStack_isFull(const Scopes *stack);
void scopeStack_push(Scopes *stack, SymTableNode *scope);
SymTableNode *scopeStack_pop(Scopes *stack);
SymTableNode *scopeStack_top(Scopes *stack);
void scopeStack_replaceTop(Scopes *stack, SymTableNode *newTop);

int symTable_searchInScopes(Scopes *stack, char id[]);

#endif // SYMTABLE