//////////////////////////////////////////////
// filename: semantic.c                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/symtable.h"
#include "../include/ast.h"

/**
 * @brief Perform semantic analysis on the AST.
 *
 * @param root pointer to the root of the AST.
 */
void semantic(ASTptr root) {
    switch(root->type) {
        case AST_PROGRAM:
            for(int i = 0; i < root->program.funcsCount; i++) {
                semantic(root->program.funcs[i]);
            }
            break;
        case AST_FUNC_DEF:
            // TODO
            break;
        // TODO
        default:
            // unexpected node type for now
            break;
    }
}
