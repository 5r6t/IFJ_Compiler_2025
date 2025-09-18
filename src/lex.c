//////////////////////////////////////////////
// filename: lex.c	                   	//
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "common.h"
#include "lex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file scanner.c
 * @brief Scans the input file and generates tokens.
 *
 * Implements a finite state machine (FSM) to tokenize the input from a file.
 * It reads characters, transitions between states, and creates tokens based 
 * on recognized patterns (e.g., identifiers, strings, operators, special characters...).
 *
 * @param file Pointer to the file to scan.
 * @return 
 * - `0` if a token was successfully created.
 * - `EOF` if the end of the file was reached.
 * - `program_error` on lexical or internal errors.
 *
 * @note The caller is responsible for handling the returned tokens and managing 
 *       memory associated with the token structure. 
 *       Ensure that the file pointer is valid and opened in read mode.
 */

 TokenPtr lexer(FILE *file) {
    TokenPtr new_token = (TokenPtr)malloc(sizeof(struct Token));

    if (new_token == NULL) {
        DEBUG_PRINT("Memory allocation error\n");
        return 1;
    }

    int state = START;
    int c;
    char ch; // buffer character
    char buffer[MAX_BUFFER_LENGTH];
    size_t pos = 0; // buffer index
    int my_int;

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
                return ERR_LEX; // Update later
            }

        // case STATE: ...
        default:
            break;
        }
    }

    return NULL;
 }