#include "stack.h"

#include "stdlib.h"

/**
 * Initializes the stack by setting the top index to -1 (empty stack).
 *
 * @param stack Pointer to the stack structure to initialize.
 */
void stack_token_init(stack_token *stack) {
    stack->top = -1; // Stack is empty when top is -1
}

/**
 * Pushes a token onto the stack.
 *
 * If the stack is full, prints a warning message.
 *
 * @param stack Pointer to the stack structure.
 * @param item Pointer to the token to push.
 */
void stack_token_push(stack_token *stack, TokenPtr item) {
    if (stack->top == MAXSTACK - 1) {
        DEBUG_PRINT("[W] Stack overflow\n");
    } else {
        stack->items[++stack->top] = item;
    }
}

/**
 * Returns the token at the top of the stack without removing it.
 *
 * If the stack is empty, returns NULL.
 *
 * @param stack Pointer to the stack structure.
 * @return Pointer to the token at the top, or NULL if the stack is empty.
 */
TokenPtr stack_token_top(stack_token *stack) {
    if (stack->top == -1) { // Stack is empty
        return NULL;
    }
    return stack->items[stack->top];
}

/**
 * Removes and returns the token at the top of the stack.
 *
 * If the stack is empty, prints a warning message and returns NULL.
 *
 * @param stack Pointer to the stack structure.
 * @return Pointer to the removed token, or NULL if the stack is empty.
 */
TokenPtr stack_token_pop(stack_token *stack) {
    if (stack->top == -1) { // Stack is empty
        DEBUG_PRINT("[W] Stack underflow\n");
        return NULL;
    }
    TokenPtr top_item = stack->items[stack->top];
    stack->items[stack->top--] = NULL; // Clear the pointer and decrement top
    return top_item;
}

/**
 * Checks if the stack is empty.
 *
 * @param stack Pointer to the stack structure.
 * @return true if the stack is empty, false otherwise.
 */
bool stack_token_empty(stack_token *stack) {
    return stack->top == -1;
}

/**
 * Clears the stack and frees all tokens.
 *
 * Frees the memory for each token in the stack and resets the stack to empty.
 *
 * @param stack Pointer to the stack structure.
 */
void stack_token_clear(stack_token *stack) {
    while (!stack_token_empty(stack)) {
        TokenPtr top_token = stack_token_pop(stack);
        if (top_token != NULL) {
            free(top_token->id);
            free(top_token->data);
            free(top_token);
        }
    }
    stack->top = -1; // Reset the stack
}
