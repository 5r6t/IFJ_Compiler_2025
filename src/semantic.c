//////////////////////////////////////////////
// filename: semantic.c                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

/**
 * @file semantic.c
 * @author Jan Hájek (xhajekj00) / Wekk
 * @brief performs semantic analysis on the AST
 * 
 */

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
    funcTableFill(&globalFunc); // fill with inbuilt functions
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
    fprintf(stderr,"Semantic analysis node type: %d\n", root->type);
    switch (root->type) // dispatch based on node type
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
        // unexpected node type
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
    for (int i = 0; i < programNode->program.funcsCount; i++) // register functions
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

        if (!funcTableAdd(&globalFunc, sym)) // function redefinition
        {
            fprintf(stderr, "Error: function redefinition\n");
            exit(4);
        }
    }

    // checks for main function
    if (funcTableGet(&globalFunc, "main", 0) == NULL 
    && !funcTableGetGetter(&globalFunc, "main") 
    && !funcTableGetSetter(&globalFunc, "main"))
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
    SymTableNode *scope = NULL; // create new scope for function

    scopeStack_push(&scopeStack, scope);

    SymTableNode *currentScope = scopeStack_top(&scopeStack);

    for (int i = 0; i < node->func.paramCount; i++) // insert parameters into scope
    {
        char *paramName = node->func.paramNames[i];

        if (paramName[0] == '_' && paramName[1] == '_')
        { // parameter can't be global
            fprintf(stderr, "Error: invalid parameter name\n");
            exit(4);
        }

        if (bst_search(currentScope, paramName))
        { // parameter redefinition
            fprintf(stderr, "Error: parameter redefinition\n");
            exit(4);
        }

        currentScope = bst_insert(currentScope, paramName);
    }

    scopeStack_pop(&scopeStack);
    scopeStack_push(&scopeStack, currentScope);

    ASTptr body = node->func.body;  // manually traverse through body because of scope
    for(int i = 0; i < body->block.stmtCount; i++){
        semanticNode(body->block.stmt[i]);
    }

    scopeStack_pop(&scopeStack); // pop function scope
}

/**
 * @brief semantic analysis for block node
 *
 * @param node pointer to the block AST node
 */
void sem_block(ASTptr node)
{
    SymTableNode *scope = NULL;
    scopeStack_push(&scopeStack, scope); // create new scope for block

    for (int i = 0; i < node->block.stmtCount; i++) // traverse through statements in block
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

    if (varName[0] == '_' && varName[1] == '_') // variable can't be global
    {
        fprintf(stderr, "Error: invalid variable name\n");
        exit(4);
    }

    SymTableNode *currentScope = scopeStack_top(&scopeStack);
    if (bst_search(currentScope, varName)) // search for variable in current scope
    {
        fprintf(stderr, "Error: variable redefinition\n");
        exit(4);
    }

    currentScope = bst_insert(currentScope, varName); // insert variable into current scope
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
    fprintf(stderr,"Target name %s, target type %d\n", name, node->assign_stmt.asType);

    semanticNode(node->assign_stmt.expr); // analyze the right side of assignment

    if (name[0] == '_' && name[1] == '_')
    { // if the left side is global variable
        node->assign_stmt.asType = TARGET_GLOBAL;
        return;
    }

    int scopeIdx = symTable_searchInScopes(&scopeStack, name); // if the left side is local variable
    if (scopeIdx != -1)
    { // found local
        node->assign_stmt.asType = TARGET_LOCAL;
        return;
    }

    FuncInfo *func = funcTableGetSetter(&globalFunc, name);
    if (func != NULL && func->kind == FUNC_SETTER)
    { // found setter
        node->assign_stmt.asType = TARGET_SETTER;
        return;
    }

    fprintf(stderr, "Error: unknown identifier in assignment\n");
    exit(3); // unknown left side
}

/**
 * @brief semantic analysis for function call node
 *
 * @param node pointer to the function call AST nodex
 */
void sem_funcCall(ASTptr node)
{
    char *funcName = node->call.funcName;
    int argc = node->call.argCount;

    FuncInfo *cand = funcTableGet(&globalFunc, funcName, -1); // search only for a function name, without arity
    if (!cand)
    { // function does not exist
        fprintf(stderr, "Error: unknown function\n");
        exit(3);
    }

    FuncInfo *func = funcTableGet(&globalFunc, funcName, argc);
    if (func == NULL)
    { // function does not exist with same arity
        fprintf(stderr, "Error: function called with wrong number of arguments\n");
        exit(5);
    }

    if (func->kind == FUNC_GETTER && argc != 0)
    { // function is getter but called with arguments
        fprintf(stderr, "Error: getter function called with arguments\n");
        exit(5);
    }

    if (func->kind == FUNC_SETTER)
    { // function is setter but called with wrong number of arguments
        fprintf(stderr, "Error: function is setter and it is called with wrong number of argumemts\n");
        exit(3);
    }

    if(func->kind == FUNC_BUILTIN)
    {
        fprintf(stderr,"Checking argument types for built-in function %s\n", funcName);
        for(int i = 0; i < node->call.argCount; i++)
        {
            if(node->call.args[i]->type == AST_LITERAL) // argument is literal
            {
                LiteralType argType = node->call.args[i]->literal.liType;
                FuncParamType expectedType = func->paramTypes[i];
                if((expectedType == PARAM_TYPE_NUMBER && argType != LIT_NUMBER) ||
                   (expectedType == PARAM_TYPE_STRING && argType != LIT_STRING) )
                {
                    fprintf(stderr, "Error: function called with wrong argument types\n");
                    exit(5);
                }

            } // checking other than literal types is part of extension STATICAN
        }
    }

    node->call.callInfo = func; // linking function info to the call node

    for (int i = 0; i < argc; i++) // analyze arguments
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
        semanticNode(node->return_stmt.expr); // return expression
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
    fprintf(stderr,"Identifier name: %s, type: %d\n", idName, node->identifier.idType);

    if (symTable_searchInScopes(&scopeStack, idName) != -1) // local variable
    {   
        node->identifier.idType = ID_LOCAL;
        return;
    }

    if (idName[0] == '_' && idName[1] == '_') // global variable
    {   
        node->identifier.idType = ID_GLOBAL;
        return;
    }

    FuncInfo *getter = funcTableGetGetter(&globalFunc, idName); // getter function
    if (getter != NULL && getter->kind == FUNC_GETTER)
    {
        node->identifier.idType = ID_GETTER;
        return;
    }

    FuncInfo *func = funcTableGet(&globalFunc, idName, -1); // function
    if (func != NULL && func->kind == FUNC_NORMAL)
    {
        fprintf(stderr, "Error: function used without parentheses\n");
        exit(3);
    }

    fprintf(stderr, "Error: unknown identifier %s\n", idName);
    exit(3);
}
/**
 * @brief semantic analysis for binary operation node
 *
 * @param node pointer to the binary operation AST node
 */
void sem_binop(ASTptr node)
{
    ASTptr left = node->binop.left;

    if(node->binop.opType == BINOP_IS){ // only left operand is needed for IS operator
        semanticNode(left);
        return;
    }

    ASTptr right = node->binop.right;

    if(node->binop.opType != BINOP_IS && node->binop.opType != BINOP_EQ && node->binop.opType != BINOP_NEQ){ // null can only be used with IS, EQ and NEQ operators
        if((left->type == AST_LITERAL && left->literal.liType == LIT_NULL) ||
           (right->type == AST_LITERAL && right->literal.liType == LIT_NULL)){
            fprintf(stderr,"Error: null used with bad operator\n");
            exit(6);
        }
    }

    semanticNode(left);
    semanticNode(right); // analyze both operands
    fprintf(stderr,"Left node type: %d, Right node type: %d\n", left->type, right->type);
    
    if (left->type == AST_LITERAL && right->type == AST_LITERAL) // both operands are literals
    {
        LiteralType leftType = left->literal.liType;
        LiteralType rightType = right->literal.liType;

        switch (node->binop.opType)
        {
        case BINOP_ADD: // addition or string concatenation
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
        case BINOP_MUL: // multiplication or string repetition
            if (leftType == LIT_NUMBER && rightType == LIT_NUMBER)
            {
                return;
            }
            else if (leftType == LIT_STRING && rightType == LIT_NUMBER)
            {
                if(!is_integer(right->literal.num) || right->literal.num < 0){
                    fprintf(stderr, "Error: right operand has to be integer and non-negative\n");
                    exit(6);
                }
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
        case BINOP_EQ:
        case BINOP_NEQ:
            if (leftType == rightType)
            {
                return;
            }
            else if(leftType == LIT_NULL || rightType == LIT_NULL)
            {
                return;
            }

            fprintf(stderr, "Error: both operands has to be of the same type for equality comparison\n");
            exit(6);
        default:
            return;
        }
    }else if(left->type == AST_IDENTIFIER && right->type == AST_LITERAL){ // left is identifier, right is literal
        if(node->binop.opType == BINOP_DIV && right->literal.liType == LIT_STRING){ // string division
            fprintf(stderr, "Error: cannot divide with string\n");
            exit(6);
        }

        if(node->binop.opType == BINOP_MUL){ // string repetition
            if(right->literal.liType == LIT_NUMBER){
                return;
            }
            fprintf(stderr, "Error: right operand has to be number type for string repetition\n");
            exit(6);
        }
        return;
    }else if(left->type == AST_LITERAL && right->type == AST_IDENTIFIER){ // left is literal, right is identifier
        if(node->binop.opType == BINOP_DIV && left->literal.liType == LIT_STRING){ // string division
            fprintf(stderr, "Error: cannot divide string\n");
            exit(6);
        }
        return;
    }else if(left->type == AST_IDENTIFIER && right->type == AST_IDENTIFIER){ // both are identifiers
        return;
    }
    
    return;
}