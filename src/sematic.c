//////////////////////////////////////////////
// filename: semantic.c                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/semantic.h"
#include "../include/ast.h"
#include "../include/symtable.h"
#include "../include/functionTable.h"

extern FuncTable globalFunc;
extern Scopes *scopeStack;

/**
 * @brief Perform semantic analysis on the AST.
 *
 * @param root pointer to the root of the AST.
 */
void semantic(ASTptr root) {
    funcTableInit(&globalFunc); // initialize function table
    scopeStack_init(scopeStack); // initialize scope stack
    registerFunctions(root); // register functions

    semanticNode(root); // start semantic analysis from root
}

void semanticNode(ASTptr root){
    switch(root->type) {
        case AST_PROGRAM:
            for(int i = 0; i < root->program.funcsCount; i++) {
                semanticNode(root->program.funcs[i]);
            }
            break;
        case AST_FUNC_DEF:
            checkFunctionDefinition(root);
            break;
        case AST_FUNC_CALL:
            // TODO
            break;
        case AST_BLOCK:
            // TODO
            break;
        case AST_IF_STMT:
            // TODO
            break;
        case AST_RETURN_STMT:
            // TODO
            break;
        default:
            // unexpected node type for now
            break;
    }
}

/**
 * @brief Check and register functions in the function table
 * 
 * @param programNode pointer to the program AST node
 */
void registerFunctions(ASTptr programNode){
    for (int i = 0; i < programNode->program.funcsCount; i++) {
        ASTptr func = programNode->program.funcs[i];

        FuncInfo sym;
        sym.name = func->func.name;
        sym.paramCount = func->func.paramCount;
        sym.funcNode = func;
        if(func->func.isGetter){
            sym.kind = FUNC_GETTER;
        }else if(func->func.isSetter) {
            sym.kind = FUNC_SETTER;
        }else {
            sym.kind = FUNC_NORMAL;
        }

        if (!funcTableAdd(&globalFunc, sym)) {
            // ERROR - function redefiniton - TODO error handling
            exit(4);
        }
    }

    // checks for main function
    if (funcTableGet(&globalFunc, "main", 0) == NULL) {
        // ERROR - main function not found - TODO error handling
        exit(3);
    }
}

/**
 * @brief Check function definition for semantic correctness
 * 
 * @param programNode pointer to the program AST node
 */
void checkFunctionDefinition(ASTptr programNode){
    SymTableNode *scope = NULL;
    scopeStack_push(scopeStack, scope);

    for(int i = 0; i < programNode->func.paramCount; i++){
        char *paramName = programNode->func.paramNames[i];
        if(bst_search(scope, paramName)){
            // ERROR - parameter redefinition - TODO error handling
            exit(4);
        }
        scope = bst_insert(scope, paramName);
        scopeStack->array[scopeStack->topIndex] = scope;
    }

    semanticNode(programNode->func.body);
    scopeStack_pop(scopeStack);
}