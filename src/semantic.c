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

FuncTable globalFunc;
Scopes scopeStack;

/**
 * @brief Perform semantic analysis on the AST.
 *
 * @param root pointer to the root of the AST.
 */
void semantic(ASTptr root) {
    funcTableInit(&globalFunc); // initialize function table
    scopeStack_init(&scopeStack); // initialize scope stack
    registerFunctions(root); // register functions

    semanticNode(root); // start semantic analysis from root
}

void semanticNode(ASTptr root){
    printf("Semantic analysis node type: %d\n", root->type);
    switch(root->type) {
        case AST_PROGRAM:
            for(int i = 0; i < root->program.funcsCount; i++) {
                semanticNode(root->program.funcs[i]);
            }
            break;
        case AST_FUNC_DEF:
            checkFunctionDefinition(root);
            break;
        case AST_BLOCK:
            sem_block(root);
            break;
        case AST_VAR_DECL:
            sem_varDecl(root);
            break;
        case AST_ASSIGN_STMT:
            sem_assignStmt(root);
            break;
        case AST_FUNC_CALL:
            sem_funcCall(root);
            break;
        case AST_IF_STMT:
            sem_ifStmt(root);
            break;
        case AST_WHILE_STMT:
            sem_whileStmt(root);
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
        fprintf(stderr, "Error: main function not found\n");
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
    scopeStack_push(&scopeStack, scope);

    for(int i = 0; i < programNode->func.paramCount; i++){
        char *paramName = programNode->func.paramNames[i];

        if(paramName[0] == '_' && paramName[1] == '_'){ // parameter can't be global
            // ERROR - invalid parameter name - TODO error handling
            fprintf(stderr, "Error: invalid parameter name\n");
            exit(4);
        }

        SymTableNode *currentScope = scopeStack_top(&scopeStack);
        if(bst_search(currentScope, paramName)){
            // ERROR - parameter redefinition - TODO error handling
            fprintf(stderr, "Error: parameter redefinition\n");
            exit(4);
        }

        currentScope = bst_insert(currentScope, paramName);
        scopeStack_pop(&scopeStack);
        scopeStack_push(&scopeStack, currentScope);
    }

    semanticNode(programNode->func.body);
    scopeStack_pop(&scopeStack);
}

/**
 * @brief semantic analysis for block node
 * 
 * @param block pointer to the block AST node
 */
void sem_block(ASTptr block){
    SymTableNode *scope = NULL;
    scopeStack_push(&scopeStack, scope);

    for(int i = 0; i < block->block.stmtCount; i++){
        semanticNode(block->block.stmt[i]);
    }

    scopeStack_pop(&scopeStack);
}

/**
 * @brief semantic analysis for variable declaration node
 * 
 * @param varDecl pointer to the variable declaration AST node
 */
void sem_varDecl(ASTptr varDecl){
    char *varName = varDecl->var_decl.varName;

    if(varName[0] == '_' && varName[1] == '_'){ // variable can't be global
        // ERROR - invalid variable name - TODO error handling
        exit(4);
    }

    SymTableNode *currentScope = scopeStack_top(&scopeStack);
    if(bst_search(currentScope, varName)){
        // ERROR - variable redefinition - TODO error handling
        exit(4);
    }

    currentScope = bst_insert(currentScope, varName);
    scopeStack_pop(&scopeStack);
    scopeStack_push(&scopeStack, currentScope);
}

/**
 * @brief semantic analysis for assignment statement node
 * 
 * @param assignStmt pointer to the assignment statement AST node
 */
void sem_assignStmt(ASTptr assignStmt){
    char *name = assignStmt->assign_stmt.targetName;

    semanticNode(assignStmt->assign_stmt.expr);

    if(name[0] == '_' && name[1] == '_'){ // if the left side is global variable
        return;
    }

    int scopeIdx = symTable_searchInScopes(&scopeStack, name); // if the left side is local variable
    if(scopeIdx != -1){ // found local
        return;
    }

    FuncInfo *func = funcTableGet(&globalFunc, name, 1); // if the left side is setter
    if(func != NULL && func->kind == FUNC_SETTER){ // found setter
        return;
    }

    exit(3); // unknown left side
}

/**
 * @brief semantic analysis for function call node
 * 
 * @param funcCall pointer to the function call AST node
 */
void sem_funcCall(ASTptr funcCall){
    char *funcName = funcCall->call.funcName;
    int argc = funcCall->call.argCount;

    FuncInfo *cand = funcTableGet(&globalFunc, funcName, -1); // search only for a function name, without arity
    if(!cand){ // function does not exist
        exit(3);
    }

    FuncInfo *func = funcTableGet(&globalFunc, funcName, argc);
    if(func == NULL){ // function does not exist with same arity
        exit(5);
    }

    if(func->kind == FUNC_GETTER && argc != 0){ // function is getter but called with arguments
        exit(5);
    }

    if(func->kind == FUNC_SETTER){ // function is setter but called with wrong number of arguments
        exit(3);
    }

    funcCall->call.callInfo = func; // linking function info to the call node

    for(int i = 0; i < argc; i++){
        semanticNode(funcCall->call.args[i]);
    }
}

/**
 * @brief semantic analysis for if statement node
 * 
 * @param node pointer to the if statement AST node
 */
void sem_ifStmt(ASTptr node){
    semanticNode(node->ifstmt.cond); // condition

    if(node->ifstmt.then != NULL){ // then node
        semanticNode(node->ifstmt.then);
    }

    if(node->ifstmt.elsestmt != NULL){ // else node
        semanticNode(node->ifstmt.elsestmt);
    }
}

/**
 * @brief semantic analysis for while statement node
 * 
 * @param node pointer to the while statement AST node
 */
void sem_whileStmt(ASTptr node){
    semanticNode(node->while_stmt.cond); // condition

    if(node->while_stmt.body != NULL){
        semanticNode(node->while_stmt.body); // body
    }
}

