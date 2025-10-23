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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    AST_PROGRAM,
    AST_FUNC_DEF,
    AST_BLOCK,
    AST_IF_STMT
} ASTnodeType;

/**
 * @brief Abstract Syntax Tree Node
 * 
 */
typedef struct ASTnode {
    ASTnodeType type;
    union {
        struct { // Program (root)
            struct ASTnode **child;
            int childCount;
        } program;

        struct {
            char *name; // function name
            char **paramNames; // parameter array
            int paramCount; // number of parameters, 0 - getter, 1 - setter, n - function
            struct ASTnode *params;
            struct ASTnode *body;
        } func;

        struct {
            struct ASTnode **child; // array of statements
        } block;

        struct { // If statement
            struct ASTnode *cond;
            struct ASTnode *then;
            struct ASTnode *els;
        } ifstmt;
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
    program_node->program.child = NULL;
    program_node->program.childCount = 0;
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