//////////////////////////////////////////////
// filename: ast.h                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

/**
 * @file ast.h
 * @brief Header file for the syntactic analyzer.
 *
 */

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct FuncInfo;
typedef struct FuncInfo FuncInfo;

typedef enum {
    AST_PROGRAM,
    AST_FUNC_DEF,
    AST_FUNC_CALL,
    AST_BLOCK,
    AST_IF_STMT,
    AST_RETURN_STMT,
    AST_VAR_DECL,
    AST_ASSIGN_STMT,
    AST_WHILE_STMT,
    AST_IDENTIFIER,
    AST_LITERAL,
    AST_BINOP
} ASTnodeType;

typedef enum {
    TARGET_LOCAL,
    TARGET_GLOBAL,
    TARGET_SETTER
} AssignTargetType;

typedef enum {
    ID_LOCAL,
    ID_GLOBAL,
    ID_GETTER
} IdType;

typedef enum {
    LIT_NULL,
    LIT_NUMBER,
    LIT_STRING
} LiteralType;

typedef enum {
    TYPE_STRING,
    TYPE_NUMBER,
    TYPE_NULL,
} TypeName;

typedef enum {
    BINOP_ADD, // + -- for strings too (concatenation)
    BINOP_SUB, // -
    BINOP_MUL, // * -- for strings too (repetition)
    BINOP_DIV, // /
    BINOP_LT, // <
    BINOP_GT, // >
    BINOP_EQ, // ==
    BINOP_NEQ, // !=
    BINOP_LTE, // <=
    BINOP_GTE, // >=
    BINOP_AND, // && -- not used (extension)
    BINOP_OR, // || -- not used (extension)
    BINOP_IS // is 
} BinOpType;

/**
 * @brief Abstract Syntax Tree Node
 * 
 */
typedef struct ASTnode {
    ASTnodeType type;
    union {
        struct { // Program (root)
            struct ASTnode **funcs;
            int funcsCount;
            int funcsCap;
        } program;

        struct {
            char *name; // function name
            char **paramNames; // parameter array
            int paramCount; // number of parameters, 0 - getter, 1 - setter, n - function
            bool isSetter; // true if setter function
            bool isGetter; // true if getter function
            struct ASTnode *body;
        } func;

        struct {
            char *funcName;
            struct ASTnode **args; // array of argument expressions
            int argCount;
            int argCap;
            FuncInfo *callInfo; // pointer to function info in function table
        } call;

        struct {
            struct ASTnode **stmt; // array of statements
            int stmtCount;
            int stmtCap;
        } block;

        struct { // if statement
            struct ASTnode *cond;
            struct ASTnode *then;
            struct ASTnode *elsestmt;
        } ifstmt;

        struct { // return statement
            struct ASTnode *expr;
        } return_stmt;

        struct { // variable declaration
            char *varName;
        } var_decl;

        struct { // assignment statement
            char *targetName;
            AssignTargetType asType;
            struct ASTnode *expr;
        } assign_stmt;

        struct { // while statement
            struct ASTnode *cond;
            struct ASTnode *body;
        } while_stmt;

        struct { // identifier
            char *name;
            IdType idType;
        } identifier;

        struct { // literal
            LiteralType liType;
            union {
                double num;
                char *str;
            };
        } literal;

        struct { // binary operation
            BinOpType opType;
            TypeName resultType; // using only when opType is BINOP_IS
            struct ASTnode *left;
            struct ASTnode *right;
        } binop;
    };
} ASTnode;

typedef ASTnode *ASTptr;

ASTptr ast_program();
ASTptr ast_function(char *name, char **paramNames);

/* 
// add this to parser.c when discussed with team
// #NEXT_MEETING
ASTptr ast_program(){
    ASTptr program_node = (ASTptr)malloc(sizeof(struct ASTnode));
    program_node->type = AST_PROGRAM;
    program_node->program.stms = NULL;
    program_node->program.stmtCount = 0;
    return program_node;
}

ASTptr ast_function(char *name, char **paramNames){
    ASTptr func_node = (ASTptr)malloc(sizeof(struct ASTnode));
    func_node->type = AST_FUNC_DEF;
    func_node->func.name = name;
    func_node->func.paramNames = paramNames;
    return func_node;
}
*/

#endif // AST_H