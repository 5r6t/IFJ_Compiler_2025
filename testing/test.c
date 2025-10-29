#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "lex.h"
#include "stack.h"

enum test_type {test_lex = 0, test_parser = 1, test_stack = 15};

void print_help(void) {
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|                   GENERAL_TEST USAGE                 |\n");
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|  Call this program with the following arguments:     |\n");
    DEBUG_PRINT("|                                                      |\n");
    DEBUG_PRINT("|  scanner = 0   -> Test the lex    functionality      |\n");
    DEBUG_PRINT("|  parser  = 1   -> Test the parser functionality      |\n");
    DEBUG_PRINT("|  stack   = 15  -> Test the stack  functionality      |\n");
    DEBUG_PRINT("|                                                      |\n");
    DEBUG_PRINT("|  Example: ./GENERAL_TEST 0 test.txt                  |\n");
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|______________________________________________________|\n");
}


void parser_test (FILE *file) {
    (void)file;
    return;
}

/**
 * @brief Tests the lex functionality.
 */
void lex_test(FILE *file) {
    DEBUG_PRINT("\n[SCANNER_TEST] Starting token scanning...\n");
    TokenPtr input_token = lexer(file);
    int token_count = 0;

    while (input_token->type != EOF) {
        token_print(input_token);
        token_free(input_token);
        input_token = lexer(file);
        token_count++;
    }
    token_print(input_token); // print EOF token
    token_count++;
    DEBUG_PRINT("\n[SCANNER_TEST] Total tokens scanned: %d\n", token_count);
}

/**
 * @brief Tests the functionality of stack_token
 */
void stack_test(void) {
    stack_token stack;
    stack_token_init(&stack);

    DEBUG_PRINT("\n[STACK_TEST] Creating tokens...\n\n");
    TokenPtr test[5];

    // Create tokens
    for (int i = 0; i < 5; i++) {
        TokenPtr new_token = token_init();

        char id[20];
        snprintf(id, sizeof(id), "STACK_TOKEN_%d", i);
        token_update(new_token, id, NULL, i);
        test[i] = new_token;
        DEBUG_PRINT("[INFO] Created token: ID: '%s', TYPE: %d\n", new_token->id, new_token->type);
    }

    // Push tokens onto the stack
    DEBUG_PRINT("\n[STACK_TEST] Pushing tokens to the stack...\n\n");
    for (int i = 0; i < 5; i++) {
        stack_token_push(&stack, test[i]);
        test[i] = NULL;
        DEBUG_PRINT("[INFO] Pushed token: ID: '%s', TYPE: %d\n", stack_token_top(&stack)->id, stack_token_top(&stack)->type);
    }

    // Print and pop tokens from the stack
    DEBUG_PRINT("\n[STACK_TEST] Printing and popping tokens from the stack:\n\n");
    while (!stack_token_empty(&stack)) {
        TokenPtr top_token = stack_token_pop(&stack);
        if (top_token != NULL) {
            token_print(top_token);
            token_free(top_token);
        }
    }

    // Clean up remaining tokens that were never pushed to the stack
    DEBUG_PRINT("\n[STACK_TEST] Cleaning up remaining tokens...\n\n");
    for (int i = 0; i < 5; i++) {
        if (test[i] != NULL) {
            token_free(test[i]);
            test[i] = NULL;
        }
    }

    DEBUG_PRINT("\n[STACK_TEST] Completed successfully!\n\n");
}

/**
 * @brief Main function to execute the appropriate test.
 */
int main(int argc, char **argv) {
    if (argc < 3) {
        print_help();
        return 1;
    }
    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        return 1;
    }

    switch (atoi(argv[1])) {
        case test_lex:
            fprintf(stdout, "\n");
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                        LEX TEST                      |\n");
            fprintf(stdout, "========================================================\n\n");

            lex_test(file);
            fclose(file);

            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                    LEX TEST COMPLETE                 |\n");
            fprintf(stdout, "========================================================\n\n");
            break;

        case test_parser:
            fprintf(stdout, "\n");
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                   PARSER TEST                        |\n");
            fprintf(stdout, "========================================================\n\n");
            fprintf(stdout, "[INFO] Parser test not yet implemented.\n"); 
            parser_test(file);
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                PARSER TEST COMPLETE                  |\n");
            fprintf(stdout, "========================================================\n\n");
            break;
        
        case test_stack:
            fprintf(stdout, "\n");
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                    STACK TEST                        |\n");
            fprintf(stdout, "========================================================\n\n");
            stack_test();
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                   SNACK TEST COMPLETE                  |\n");
            fprintf(stdout, "========================================================\n\n");
            break;
    
        default:
            fprintf(stderr, "\n");
            fprintf(stderr, "========================================================\n");
            fprintf(stderr, "|                  INVALID ARGUMENT                    |\n");
            fprintf(stderr, "========================================================\n\n");
            print_help();
            break;
    }
    return 0;
}
