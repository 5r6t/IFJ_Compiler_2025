//////////////////////////////////////////////
// filename: semantic.h                	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

void semantic(ASTptr root);
void semanticNode(ASTptr root);
void registerFunctions(ASTptr programNode);
void sem_funcDef(ASTptr node);
void sem_block(ASTptr node);
void sem_varDecl(ASTptr node);
void sem_assignStmt(ASTptr node);
void sem_funcCall(ASTptr node);
void sem_ifStmt(ASTptr node);
void sem_whileStmt(ASTptr node);
void sem_returnStmt(ASTptr node);
void sem_identifier(ASTptr node);
void sem_binop(ASTptr node);

#endif // SEMANTIC_H