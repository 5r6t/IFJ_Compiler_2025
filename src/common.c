//////////////////////////////////////////////
// filename: common.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/common.h"
#include <stdlib.h>
#include <string.h>

/**
 * @file common.c
 * @brief Utility functions for token management and error handling.
 *
 * This file contains implementations for:
 * - Token initialization, creation, and printing.
 * - String duplication.
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
 * @brief Creates a token and initializes its fields.
 *
 * Sets the token's ID, data, and type. The token's fields are dynamically
 * allocated copies of the provided strings.
 *
 * @param token Pointer to the token to initialize.
 * @param id String for the token ID.
 * @param data String for the token data.
 * @param type Integer representing the token type.
 */
void token_create(TokenPtr token, const char *id, const char *data, int type)
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

    DEBUG_PRINT("[TOKEN] ID: '%10s' | TYPE: %3d | DATA: '%s'\n",
                token->id ? token->id : "N/A",
                token->type,
                token->data ? token->data : "N/A");
}

char *error_list[] = {
    "\n!!! Internal error has occurred within the program. !!!\n",
    "\n!!! Invalid token has been found during scanning process: !!!\n",
    "\n!!! String has reached the implementation limit !!!\n", // MAX_BUFFER_LENGTH
    "\nProblem has occurred during scanning. Logic not implemented yet? Problem with:\n"};

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
        token_free(bad_token);
    }
    fclose(file);
    exit(err_type);
}