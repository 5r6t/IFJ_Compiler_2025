//////////////////////////////////////////////
// filename: lex_test_data.c                //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
// * Jozef Matus (xmatusj00) / karfisk 	    //
// * Jan Hajek (xhajekj00) / Wekk 	        //
//////////////////////////////////////////////

/**
 * @file lex_test_data.h
 * @brief Header for numeric lexer test data
 */

#ifndef LEX_TEST_DATA_H
#define LEX_TEST_DATA_H

#include "../../include/lex.h"

/**
 * @brief Single lexer test case
 */
typedef struct {
    char *input;          ///< Input to feed to lexer
    struct Token *expected; ///< Expected tokens (terminated by type = -1)
} LexCase;

typedef struct {
    char *name;      // e.g. "num", "string"
    LexCase *good_cases;   // pointer to good LexCase array
    LexCase *bad_cases;    // pointer to bad LexCase array
} LexGroup;

extern LexGroup lex_groups[];
extern const int LEX_GROUP_COUNT;

#endif