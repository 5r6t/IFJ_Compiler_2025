//////////////////////////////////////////////
// filename: stack.h                   	    //
// IFJ_prekladac	varianta - vv-BVS   	    //
// This data structure was taken from IAL   //
// presentations.                           //
//////////////////////////////////////////////

#ifndef IAL_STACK_H
#define IAL_STACK_H

#include <stdio.h>
#include <ast.h>
#include <stdbool.h>
#include <common.h>

#define MAXSTACK 256

typedef struct
{
  TokenPtr token;
  ASTptr ast;
} stack_item;

typedef struct
{
  stack_item items[MAXSTACK];
  int top;
} stack_token;

void stack_token_init(stack_token *stack);

void stack_token_push(stack_token *stack, TokenPtr item, ASTptr ptr);

stack_item stack_token_pop(stack_token *stack);

stack_item stack_token_top(stack_token *stack);

bool stack_token_empty(stack_token *stack);

void stack_token_clear(stack_token *stack);

void stack_token_insert_after(stack_token *stack, int a_index, TokenPtr token, ASTptr ast);

#endif