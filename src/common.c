//////////////////////////////////////////////
// filename: common.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "common.h"
#include "lex.h"

#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * @file common.c
 * @brief Utility functions for token management and error handling.
 *
 * This file contains implementations for:
 * - Token initialization, creation, and printing.
 * - String duplication, regex checks
 * - Error handling and program termination.
 */

/**
 * @brief Duplicates a string.
 *
 * Allocates memory and copies the content of the input string.
 * The caller must free the returned pointer.
 *
 * @param str Input string to duplicate. If `NULL`, returns `NULL`.
 * @return Pointer to the duplicated string, or `NULL` on failure.
 */
char *my_strdup(const char *str)
{
    if (str == NULL)
        return NULL;
    size_t length = strlen(str) + 1;
    char *dup = malloc(length);
    if (dup != NULL)
    {
        strcpy(dup, str);
    }
    return dup;
}

/**
 * @brief Allocates memory, initializes and returns and empty token.
 */
TokenPtr token_init()
{
    TokenPtr new_token = (TokenPtr)malloc(sizeof(struct Token));
    if (new_token == NULL)
    {
        DEBUG_PRINT("Memory allocation error\n");
        return NULL;
    }

    new_token->data = NULL;
    new_token->id = NULL;
    new_token->type = -1; // invalid type
    return new_token;
}

/**
 * @brief Tries to match a string against a provided regex pattern (string)
 * @return 0 on success
 */
int regex_match(const char *string, const char *pattern)
{
    regex_t regex;
    int ret;

    // compile regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret)
        program_error(NULL, ERR_INTERNAL, ERR_MSG_INTERNAL, NULL);

    // execute regex
    ret = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);

    return ret; // 0 == match
}

/**
 * @brief Creates a token and initializes its fields.
 *
 * Token's fields are dynamically allocated copies of the provided strings.
 *
 * @param token Pointer to the token to initialize.
 * @param id String for the token ID.
 * @param data String for the token data.
 * @param type Integer representing the token type.
 */
void token_update(TokenPtr token, const char *id, const char *data, int type)
{
    token->id = my_strdup(id);
    token->data = my_strdup(data);
    token->type = type;
    // DEBUG_PRINT("Token created: ID=%s, TYPE=%d\n", id ? id : "NULL", type);
}

/**
 * @brief Frees the memory allocated for a token and its fields.
 *
 * @param token Pointer to the token to be freed.
 */
void token_free(TokenPtr token)
{
    if (token != NULL)
    {
        if (token->id != NULL)
        {
            free(token->id);
            token->id = NULL;
        }
        if (token->data != NULL)
        {
            free(token->data);
            token->data = NULL;
        }
        // Free the token itself
        free(token);
    }
}

// TABLE for printing token types as strings, not numbers
static const char *names[] = {
    [START] = "START",
    [IDENTIFIER] = "IDENTIFIER",
    [ID_GLOBAL_VAR] = "ID_GLOBAL_VAR",
    [NUMERICAL] = "NUMERICAL",
    [STRING] = "STRING",
    [MULTILINE_STRING] = "MULTILINE_STRING",
    [STRING_SPECIAL] = "STRING_SPECIAL",

    [CMP_OPERATOR] = "CMP_OPERATOR",
    [ARITHMETICAL] = "ARITHMETICAL",

    [SPECIAL] = "SPECIAL",
    [NEWLINE] = "NEWLINE",

    [COMMENT] = "COMMENT",
    [BLOCK_COMMENT] = "BLOCK_COMMENT",

    [KW_CLASS] = "KW_CLASS",
    [KW_ELSE] = "KW_ELSE",
    [KW_FOR] = "KW_FOR",
    [KW_IF] = "KW_IF",
    [KW_IFJ] = "KW_IFJ",
    [KW_IMPORT] = "KW_IMPORT",
    [KW_IS] = "KW_IS",
    [KW_NULL] = "KW_NULL",           // "null" literal
    [KW_NULL_TYPE] = "KW_NULL_TYPE", // "Null" type
    [KW_NUM] = "KW_NUM",
    [KW_RETURN] = "KW_RETURN",
    [KW_STATIC] = "KW_STATIC",
    [KW_STRING] = "KW_STRING",
    [KW_VAR] = "KW_VAR",
    [KW_WHILE] = "KW_WHILE"};

const char *token_type_name(int type)
{
    // Must match the defines in lex.h
    if (type == FILE_END)
        return "FILE_END";
    if (type >= 0 && type < (int)(sizeof(names) / sizeof(names[0])) && names[type])
        return names[type];
    return NULL;
}

/**
 * @brief Prints the token's details for debugging purposes.
 *
 * Outputs the token's ID, type, and data to `stderr` if debugging is enabled.
 *
 * @param token Pointer to the token to print.
 */
void token_print(TokenPtr token)
{
    if (token == NULL)
        return;

    const char *type_name = token_type_name(token->type);
    if (type_name)
        DEBUG_PRINT("[TOKEN] ID: '%12s' | TYPE: %15s | DATA: '%s'\n",
                    token->id ? token->id : "N/A",
                    type_name,
                    token->data ? token->data : "N/A");
    else
        DEBUG_PRINT("[TOKEN] ID: '%12s' | TYPE: %3d | DATA: '%s'\n",
                    token->id ? token->id : "N/A",
                    token->type,
                    token->data ? token->data : "N/A");
}

/// @brief suggestion -- add defines for error messages?
char *error_list[] = {
    "\n!!! Internal error has occurred within the program. !!!\n",
    "\n!!! Invalid token has been found during scanning process: !!!\n",
    "\n!!! String has reached the implementation limit !!!\n", // MAX_BUFFER_LENGTH
    "\n!!! Problem has occurred during scanning. Logic not implemented yet? Problem with:\n",
    "\n!!! Error: wrong syntax found (Tak tohle je spatne a ja te zabiju) \n",
    "\n!!! Unenclosed comment found - Valve please fix. !!! "};

/**
 * @brief Closes a file and exits the program with an error code.
 *
 * Used to handle errors by closing the supplied file, printing an error
 * message to `stderr`, and terminating the program.
 *
 * @param file Pointer to the file to close.
 * @param err_type Error code for program termination.
 */
void program_error(FILE *file, int err_type, int err_out, TokenPtr bad_token)
{
    fprintf(stderr, "%s\n", error_list[err_out]);
    if (bad_token != NULL)
    {
        token_print(bad_token);
        // token_free(bad_token);
    }
    fclose(file);
    exit(err_type);
}