//////////////////////////////////////////////
// filename: lex.c	                   	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/common.h"
#include "../include/lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @file lex.c
 * @brief Scans the input file and generates tokens.
 *
 * Implements a finite state machine (FSM) to tokenize the input from a file.
 * It reads characters, transitions between states, and creates tokens based 
 * on recognized patterns (e.g., identifiers, strings, operators, special characters...).
 *
 * @param file Pointer to the file to scan.
 * @return 
 * - `EOF` in token->value if the end of the file was reached.
 * - `program_error` on lexical or internal errors.
 *
 * @note The caller is responsible for handling the returned tokens and managing 
 *       memory associated with the token structure. 
 *       Ensure that the file pointer is valid and opened in read mode.
 */

// Helper function to safely append a character to the buffer and increment the position
static void buffer_append(char *buffer, size_t *pos, int c, FILE *file, TokenPtr token) {
    if (*pos >= MAX_BUFFER_LENGTH - 1) {
        buffer[*pos] = '\0';
        program_error(file, ERR_INTERNAL, 2, token);
    }
    buffer[*pos] = c;
    (*pos)++;
}

TokenPtr lexer(FILE *file) {
    
    TokenPtr new_token =  token_init();

    int state = START;
    int c;
    char buffer[MAX_BUFFER_LENGTH];
    size_t pos = 0; // buffer index
    int my_int;
    
    (void) my_int; // to avoid unused variable warning for now


    c = fgetc(file);

    while(c != EOF) {
        switch (state)
        {
        // ignore whitespace for now
        case (START):
            if (isspace(c)) {
                c = fgetc(file);
                continue;
            }
            else if (strchr("(){}[]|@;:,.?'", c)) {
                state = SPECIAL;
            }
            else if (strchr("<=>", c)) {
                state = CMP_OPERATOR;
            }
            else if (c == '!') {
                state = NOT_EQUAL;
            }
            else if (strchr("/+-*", c)) {
                state = ARITHMETICAL;
            }
            else if (c == '\"') {
                state = STRING;
            }
            else if (isalpha(c) || c == '_') {
                state = IDENTIFIER;
            }
            else if (isdigit(c)) {
                state = our_INT; // can become float later
            }
            else if (c == '\\') {
                state = MULTILINE_STRING_1;
            }
            else {
                program_error(file, ERR_LEX, 0, NULL);
            }
            break;

        // case STATE: ...
        case IDENTIFIER:
            if (isalnum(c) || c == '_') {
                buffer_append(buffer, &pos, c, file, new_token); // build identifier
                c = fgetc(file);
            } else {
                buffer[pos] = '\0';
                ungetc(c, file);
                token_create(new_token, buffer, NULL, IDENTIFIER);
                return new_token;
            }
            break;

        default:
            break;
        }
    }
    token_create(new_token, NULL, NULL, EOF);
    return new_token;
}