//////////////////////////////////////////////
// filename: ast.h                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct ASTnode {
    int type;
    union {
        struct { // Program (root)
            struct ASTnode **child;
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
