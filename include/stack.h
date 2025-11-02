#ifndef IAL_STACK_H
#define IAL_STACK_H

#include <stdio.h>
#include <stdbool.h>
#include <common.h>

#define MAXSTACK 256

  typedef struct {
    TokenPtr items[MAXSTACK];                                            
    int top;                                                                   
  } stack_token;                                                                
                                                                    
  void stack_token_init(stack_token *stack);    

  void stack_token_push(stack_token *stack, TokenPtr item); 

  TokenPtr stack_token_pop(stack_token *stack);  

  TokenPtr stack_token_top(stack_token *stack);  

  bool stack_token_empty(stack_token *stack);

  void stack_token_clear(stack_token *stack);
  
#endif