//////////////////////////////////////////////
// filename: semantic.c                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "semantic.h"
#include "ast.h"
#include "symtable.h"
#include "functionTable.h"

FuncTable globalFunc;
Scopes scopeStack;

/**
 * @brief perform semantic analysis on the AST
 *
 * @param root pointer to the root of the AST
 */
void semantic(ASTptr root)
{
    funcTableInit(&globalFunc);   // initialize function table
    scopeStack_init(&scopeStack); // initialize scope stack
    registerFunctions(root);      // register functions

    semanticNode(root); // start semantic analysis from root
}

/**
 * @brief main function of semantic analysis. Performs semantic checks based on node type
 *
 * @param root pointer to the AST node
 */
void semanticNode(ASTptr root)
{
    printf("Semantic analysis node type: %d\n", root->type);
    switch (root->type)
    {
    case AST_PROGRAM:
        for (int i = 0; i < root->program.funcsCount; i++)
        {
            semanticNode(root->program.funcs[i]);
        }
        break;
    case AST_FUNC_DEF:
        sem_funcDef(root);
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
        sem_returnStmt(root);
        break;
    case AST_IDENTIFIER:
        sem_identifier(root);
        break;
    case AST_BINOP:
        sem_binop(root);
        break;
    default:
        // unexpected node type for now
        break;
    }
}

/**
 * @brief check and register functions in the function table
 *
 * @param programNode pointer to the program AST node
 */
void registerFunctions(ASTptr programNode)
{
    for (int i = 0; i < programNode->program.funcsCount; i++)
    {
        ASTptr func = programNode->program.funcs[i];

        FuncInfo sym;
        sym.name = func->func.name;
        sym.paramCount = func->func.paramCount;
        sym.funcNode = func;
        if (func->func.isGetter)
        {
            sym.kind = FUNC_GETTER;
        }
        else if (func->func.isSetter)
        {
            sym.kind = FUNC_SETTER;
        }
        else
        {
            sym.kind = FUNC_NORMAL;
        }

        if (!funcTableAdd(&globalFunc, sym))
        {
            fprintf(stderr, "Error: function redefinition\n");
            exit(4);
        }
    }

    // checks for main function
    if (funcTableGet(&globalFunc, "main", 0) == NULL)
    {
        fprintf(stderr, "Error: main function not found\n");
        exit(3);
    }
}

/**
 * @brief check function definition for semantic correctness
 *
 * @param node pointer to the program AST node
 */
void sem_funcDef(ASTptr node)
{
    SymTableNode *scope = NULL;
    scopeStack_push(&scopeStack, scope);

    SymTableNode *currentScope = scopeStack_top(&scopeStack);

    for (int i = 0; i < node->func.paramCount; i++)
    {
        char *paramName = node->func.paramNames[i];

        if (paramName[0] == '_' && paramName[1] == '_')
        { // parameter can't be global
            fprintf(stderr, "Error: invalid parameter name\n");
            exit(4);
        }

        if (bst_search(currentScope, paramName))
        {
            fprintf(stderr, "Error: parameter redefinition\n");
            exit(4);
        }

        currentScope = bst_insert(currentScope, paramName);
    }

    scopeStack_pop(&scopeStack);
    scopeStack_push(&scopeStack, currentScope);
    semanticNode(node->func.body);
    scopeStack_pop(&scopeStack);
}

/**
 * @brief semantic analysis for block node
 *
 * @param node pointer to the block AST node
 */
void sem_block(ASTptr node)
{
    SymTableNode *scope = NULL;
    scopeStack_push(&scopeStack, scope);

    for (int i = 0; i < node->block.stmtCount; i++)
    {
        semanticNode(node->block.stmt[i]);
    }

    scopeStack_pop(&scopeStack);
}

/**
 * @brief semantic analysis for variable declaration node
 *
 * @param node pointer to the variable declaration AST node
 */
void sem_varDecl(ASTptr node)
{
    char *varName = node->var_decl.varName;

    if (varName[0] == '_' && varName[1] == '_')
    { // variable can't be global
        fprintf(stderr, "Error: invalid variable name\n");
        exit(4);
    }

    SymTableNode *currentScope = scopeStack_top(&scopeStack);
    if (bst_search(currentScope, varName))
    {
        fprintf(stderr, "Error: variable redefinition\n");
        exit(4);
    }

    currentScope = bst_insert(currentScope, varName);
    scopeStack_pop(&scopeStack);
    scopeStack_push(&scopeStack, currentScope);
}

/**
 * @brief semantic analysis for assignment statement node
 *
 * @param node pointer to the assignment statement AST node
 */
void sem_assignStmt(ASTptr node)
{
    char *name = node->assign_stmt.targetName;

    semanticNode(node->assign_stmt.expr);

    if (name[0] == '_' && name[1] == '_')
    { // if the left side is global variable
        return;
    }

    int scopeIdx = symTable_searchInScopes(&scopeStack, name); // if the left side is local variable
    if (scopeIdx != -1)
    { // found local
        return;
    }

    FuncInfo *func = funcTableGetSetter(&globalFunc, name);
    if (func != NULL && func->kind == FUNC_SETTER)
    { // found setter
        return;
    }

    fprintf(stderr, "Error: unknown identifier in assignment\n");
    exit(3); // unknown left side
}

/**
 * @brief semantic analysis for function call node
 *
 * @param node pointer to the function call AST node
 */
void sem_funcCall(ASTptr node)
{
    char *funcName = node->call.funcName;
    int argc = node->call.argCount;

    FuncInfo *cand = funcTableGet(&globalFunc, funcName, -1); // search only for a function name, without arity
    if (!cand)
    { // function does not exist
        exit(3);
    }

    FuncInfo *func = funcTableGet(&globalFunc, funcName, argc);
    if (func == NULL)
    { // function does not exist with same arity
        exit(5);
    }

    if (func->kind == FUNC_GETTER && argc != 0)
    { // function is getter but called with arguments
        exit(5);
    }

    if (func->kind == FUNC_SETTER)
    { // function is setter but called with wrong number of arguments
        exit(3);
    }

    node->call.callInfo = func; // linking function info to the call node

    for (int i = 0; i < argc; i++)
    {
        semanticNode(node->call.args[i]);
    }
}

/**
 * @brief semantic analysis for if statement node
 *
 * @param node pointer to the if statement AST node
 */
void sem_ifStmt(ASTptr node)
{
    semanticNode(node->ifstmt.cond); // condition

    if (node->ifstmt.then != NULL)
    { // then node
        semanticNode(node->ifstmt.then);
    }

    if (node->ifstmt.elsestmt != NULL)
    { // else node
        semanticNode(node->ifstmt.elsestmt);
    }
}

/**
 * @brief semantic analysis for while statement node
 *
 * @param node pointer to the while statement AST node
 */
void sem_whileStmt(ASTptr node)
{
    semanticNode(node->while_stmt.cond); // condition

    if (node->while_stmt.body != NULL)
    {
        semanticNode(node->while_stmt.body); // body
    }
}

/**
 * @brief semantic analysis for return statement node
 *
 * @param node pointer to the return statement AST node
 */
void sem_returnStmt(ASTptr node)
{
    if (node->return_stmt.expr != NULL)
    {
        semanticNode(node->return_stmt.expr);
    }
}

/**
 * @brief semantic analysis for identifier node
 *
 * @param node pointer to the identifier AST node
 */
void sem_identifier(ASTptr node)
{
    char *idName = node->identifier.name;

    switch (node->identifier.idType)
    {
    case ID_GETTER:
    {
        FuncInfo *func = funcTableGet(&globalFunc, idName, -1);
        if (func == NULL || func->kind != FUNC_GETTER)
        {
            fprintf(stderr, "Error: unknown getter function\n");
            exit(3);
        }
        node->identifier.idType = ID_GETTER;
        break;
    }
    case ID_LOCAL:
        if (symTable_searchInScopes(&scopeStack, idName) == -1)
        {
            fprintf(stderr, "Error: unknown local identifier\n");
            exit(3);
        }
        break;
    case ID_GLOBAL:
        return;
    }

    return;
}

/**
 * @brief semantic analysis for binary operation node
 *
 * @param node pointer to the binary operation AST node
 */
void sem_binop(ASTptr node)
{
    ASTptr left = node->binop.left;
    ASTptr right = node->binop.right;

    semanticNode(left);
    semanticNode(right);

    if (left->type == AST_LITERAL && right->type == AST_LITERAL)
    {
        LiteralType leftType = left->literal.liType;
        LiteralType rightType = right->literal.liType;

        switch (node->binop.opType)
        {
        case BINOP_ADD:
            if (leftType == LIT_STRING && rightType == LIT_STRING)
            {
                return;
            }
            else if (leftType == LIT_NUMBER && rightType == LIT_NUMBER)
            {
                return;
            }
            else
            {
                fprintf(stderr, "Error: both operands has to be the same type (string or number)\n");
                exit(6);
            }
            return;
        case BINOP_SUB:
        case BINOP_DIV:
            if (leftType == LIT_NUMBER && rightType == LIT_NUMBER)
            {
                return;
            }
            else
            {
                fprintf(stderr, "Error: both operands has to be number type\n");
                exit(6);
            }
            return;
        case BINOP_MUL:
            if (leftType == LIT_NUMBER && rightType == LIT_NUMBER)
            {
                return;
            }
            else if (leftType == LIT_STRING && rightType == LIT_NUMBER)
            {
                return;
            }
            else
            {
                exit(6);
            }
            return;
        case BINOP_LT:
        case BINOP_GT:
        case BINOP_LTE:
        case BINOP_GTE:
            if (leftType == LIT_NUMBER && rightType == LIT_NUMBER)
            {
                return;
            }
            else
            {
                fprintf(stderr, "Error: both operands has to be number type\n");
                exit(6);
            }
            return;
        default:
            return;
        }
    }
}