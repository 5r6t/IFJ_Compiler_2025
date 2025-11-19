//////////////////////////////////////////////
// filename: semantic.h                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../include/ast.h"

void semantic(ASTptr root);
void semanticNode(ASTptr root);
void registerFunctions(ASTptr programNode);
void checkFunctionDefinition(ASTptr programNode);
void sem_block(ASTptr block);
void sem_varDecl(ASTptr varDecl);
void sem_assignStmt(ASTptr assignStmt);
void sem_funcCall(ASTptr funcCall);
void sem_ifStmt(ASTptr node);
void sem_whileStmt(ASTptr node);

#endif // SEMANTIC_H