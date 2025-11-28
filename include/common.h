//////////////////////////////////////////////
// filename: common.h	                    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

/**
 * @file common.h
 * @brief Header file for the lexical analyzer (scanner).
 *
 * This header provides:
 * - Error code definitions.
 * - Debug macro
 * - Token structure definition and related function declarations.
 *
 * Constants:
 * - `MAX_BUFFER_LENGTH`: Maximum length for tokens data/id storage.
 * - State definitions for various token types (e.g., identifiers, strings, operators).
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#include <stdio.h>

#define ERR_LEX 1        // Lexical error: invalid token structure
#define ERR_SYN 2        // Syntax error: program structure/corpus missing
#define ERR_SEM_UNDEF 3  // Semantic: undefined function/variable
#define ERR_SEM_REDEF 4  // Semantic: redefinition of function/variable
#define ERR_SEM_ARGS 5   // Semantic: invalid arg count/type in call/builtin
#define ERR_SEM_TYPE 6   // Semantic: type mismatch in expr (arith/string/rel)
#define ERR_SEM_OTHER 10 // Other semantic errors
#define ERR_INTERNAL 99  // Internal compiler error (e.g. malloc fail)

// --- Runtime Error Codes ---
#define ERR_RUNTIME_ARG 25  // Runtime: invalid builtin parameter type
#define ERR_RUNTIME_TYPE 26 // Runtime: type mismatch in expr at runtime

// ---- Codegen Errors ----
#define ERR_CDGN_NOT_IMPLEMENTED 34

// --- ERROR MESSAGE NUMBERS FOR program_error()
#define ERR_MSG_INTERNAL 0
#define ERR_MSG_INVALID_TOK 1
#define ERR_MSG_BUFF_OVERFLOW 2
#define ERR_MSG_NOT_IMPLEMENTED 3
#define ERR_MSG_WRONG_SYNTAX 4
#define ERR_MSG_UNENCLOSED_COMM 5

typedef struct Token
{
    int type;
    char *id;
    char *data;
} *TokenPtr;

typedef struct Target
{
    int type;
    char *data;
    char *id;
} target;

TokenPtr token_init();
void token_update(TokenPtr token, const char *id, const char *data, int type);
void token_free(TokenPtr token);
void token_print(TokenPtr token);

char *my_strdup(const char *str);
int regex_match(const char *string, const char *pattern);
void program_error(FILE *file, int err_type, int err_index, TokenPtr bad_token);

#endif