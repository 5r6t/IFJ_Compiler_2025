//////////////////////////////////////////////
// filename: psaParser.h             	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jozef Matus (xmatusj00) / karfisk 	//
//////////////////////////////////////////////

#ifndef PSAPARSER_H
#define PSAPARSER_H

#include "common.h"
#include "lex.h"
#include "stack.h"

#define PRECEDENCE_ROWS 15
#define PRECEDENCE_COLS 15

typedef enum
{
    PRE_TAB_NULL = 0,
    PRE_TAB_LESS = 1,
    PRE_TAB_GREATER = 2,
    PRE_TAB_EQUAL = 3
} PrecedentTableRel;

// Postion of operators in precedence table
#define MULL 0
#define DIV 1
#define ADD 2
#define SUB 3
#define EQUAL 4
#define NEQUAL 5
#define LESS 6
#define GREATER 7
#define LSEQ 8
#define GREQ 9
#define LPAR 10
#define RPAR 11
#define ID 12
#define IS 13
#define END 14

ASTptr parse_expression(TokenPtr *nextToken, FILE *file, const target *endIfExp);
int converter(TokenPtr *tokenToConvert, FILE *file);
void checkEnd(int end, stack_token *stack, FILE *file);
ASTptr ruleIS(int index, stack_token *stack, FILE *file);
ASTptr ruleArithmetics(int index, stack_token *stack, FILE *file);
ASTptr ruleComper(int index, stack_token *stack, FILE *file);
ASTptr ruleParenthesis(int index, stack_token *stack, FILE *file);
void popRuleFromStack(int index_shift, stack_token *stack);
ASTptr checkForOtherRules(int index, stack_token *stack, FILE *file);
ASTptr checkForI(int index_shift, stack_token *stack, FILE *file);
ASTptr reduce(FILE *file, stack_token *stack);
void debug_stack(stack_token *stack);

#endif