#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"
#include "../include/lex.h"

enum test_type {test_lex, test_parser};

void print_help(void) {
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|                   GENERAL_TEST USAGE                 |\n");
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|  Call this program with the following arguments:     |\n");
    DEBUG_PRINT("|                                                      |\n");
    DEBUG_PRINT("|  scanner = 0   -> Test the lex functionality         |\n");
    DEBUG_PRINT("|  parser  = 1   -> Test the parser functionality      |\n");
    DEBUG_PRINT("|                                                      |\n");
    DEBUG_PRINT("|  Example: ./GENERAL_TEST 0 test.txt                  |\n");
    DEBUG_PRINT("|------------------------------------------------------|\n");
    DEBUG_PRINT("|______________________________________________________|\n");
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
 * @brief Main function to execute the appropriate test.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    switch (atoi(argv[1])) {
        case test_lex:
            fprintf(stdout, "\n");
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                        LEX TEST                      |\n");
            fprintf(stdout, "========================================================\n\n");
            
            if (argc < 3) {
                fprintf(stderr, "[ERROR] No input file provided for lex test.\n");
                return 1;
            }
            FILE *file = fopen(argv[2], "r");
            if (file == NULL) {
                fprintf(stderr, "[ERROR] Unable to open file: %s\n", argv[2]);
                return 1;
            }

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
            fprintf(stdout, "========================================================\n");
            fprintf(stdout, "|                PARSER TEST COMPLETE                  |\n");
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
