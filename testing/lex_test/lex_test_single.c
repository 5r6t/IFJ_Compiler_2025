//////////////////////////////////////////////
// filename: lex_test_single.c	            //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
// * Jozef Matus (xmatusj00) / karfisk 	    //
// * Jan Hajek (xhajekj00) / Wekk 	        //
//////////////////////////////////////////////

/**
 * @file test_lex_single.c
 * @brief Runs exactly one lexer test case (group, type, index)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex_test_data.h"
#include "../../include/common.h"

#define EXIT_BAD_NOEXIT    2   /* lexer accepted invalid token */
#define EXIT_MISMATCH      97
#define EXIT_INVALID_GROUP 98
#define EXIT_USAGE_ERROR   99
#define EXIT_OUT_OF_RANGE  100

int main(int argc, char **argv) {

    int lex_group_count = 0;
    for (; lex_groups[lex_group_count].name != NULL; lex_group_count++) {
        lex_group_count++;    
    }
    if (argc >= 2 && strcmp(argv[1], "GROUP_COUNT") == 0) {
        return lex_group_count;
    }

    if (argc < 4)
        return EXIT_USAGE_ERROR;

    int group_index = atoi(argv[1]);
    char *which = argv[2];
    int case_index = atoi(argv[3]);

    DEBUG_PRINT("Args: group=%d type=%s index=%d\n", group_index, which, case_index);

    if (group_index < 0 || group_index >= lex_group_count)
        return EXIT_INVALID_GROUP;

    LexGroup *grp = &lex_groups[group_index];
    LexCase *cases = (strcmp(which, "good") == 0)
                     ? grp->good_cases
                     : grp->bad_cases;

    if (!cases)
        return EXIT_OUT_OF_RANGE;

    LexCase *ptr = cases;
    for (int i = 0; i < case_index; i++) {
        if (!ptr->input)
            return EXIT_OUT_OF_RANGE;
        ptr++;
    }

    if (!ptr->input)
        return EXIT_OUT_OF_RANGE;

    char *src = ptr->input;
    FILE *f = fmemopen((void *)src, strlen(src), "r");
    if (!f) { perror("fmemopen"); return ERR_INTERNAL; }


    DEBUG_PRINT("Running against");
    token_print(ptr->expected);
    TokenPtr tok = lexer(f); // call lexer on stream
    fclose(f);
    token_print(tok);

    if (strcmp(which, "good") == 0) {
        if (ptr->expected && tok && tok->type == ptr->expected->type) {
            token_free(tok);
            return 0; // token created as expected
        }
    } else {
        /* bad case */
        if (tok) {
            DEBUG_PRINT("Lex didn't exit on bad token -> type=%d\n", tok->type);
            token_free(tok);
            return EXIT_BAD_NOEXIT; // bad case must result in exit -> FAIL
        }
    }
    token_free(tok); // token_free handles NULL safely
    return EXIT_MISMATCH;
}
