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
#include <stdbool.h>

/**
 * @file lex.c
 * @brief Scans the input file and generates tokens.
 *
 * Implements a finite state machine (FSM) to tokenize the input from a file.
 * It reads characters, transitions between states, and creates tokens based 
 * on recognized patterns (e.g., identifiers, strings, operators, special characters...).
 */

 // Helper function to skip whitespace characters and return the next non-whitespace character
int skip_whitespace(FILE *file) {
    int c = fgetc(file);

    while (c != EOF && isblank(c)) {
        c = fgetc(file);
    }

    if (c != EOF) {
        ungetc(c, file); // push back the first non-whitespace
    }
    return c; // return the non-whitespace (or EOF)
}


// Helper function to compare keywords against a string
int is_keyword(const char *str) {
    static const char *keywords [] = {
        "class", "if", "else", "is", "null", "return", "var", "while", "Ifj",
        "static", "true", "false", "Num", "String", "Null"
    };

    static size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for (size_t i = 0; i < num_keywords; i++) {
        if (strcmp(str, keywords[i]) == 0)
            return 1; // It's a keyword
    }
    return 0; // Not a keyword
}

// Helper function to safely append a character to the buffer and increment the position
static void buffer_append(char *buffer, size_t *pos, int c, FILE *file, TokenPtr token) {
    if (*pos >= MAX_BUFFER_LENGTH - 1) {
        program_error(file, ERR_INTERNAL, 2, token);
    }
    buffer[*pos] = c;
    (*pos)++;
}

/** 
 * @brief Scans the input file and generates tokens.
 * @param file Pointer to the file to scan.
 * @return 
 * - `EOF` in token->value if the end of the file was reached.
 * - `program_error` on lexical or internal errors.
 *
 * @note The caller is responsible for handling the returned tokens and managing 
 *       memory associated with the token structure. 
 *       Ensure that the file pointer is valid and opened in read mode.
 */
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
                state = OUR_INT; // can become float later
            }
            else if (c == '\\') {
                state = MULTILINE_STRING_1;
            }
            else {
                program_error(file, ERR_LEX, 0, NULL);
            }
            break;

        case IDENTIFIER:
            if (isalnum(c) || c == '_') {
                buffer_append(buffer, &pos, c, file, new_token); // build identifier
                c = fgetc(file);

            } else {
                c = skip_whitespace(file);
                if (c == '.') { // IDENTIF/KEYWORD "."  IDENTIF
                    buffer_append(buffer, &pos, c, file, new_token);
                    fgetc(file); // consume the '.'
                    c = skip_whitespace(file); // peek next non-blank for IN_BUILT_FUNC
                    state = IN_BUILT_FUNC;

                    break; // go build second part of IDENTIF
                }
                
                buffer[pos] = '\0';
                if (is_keyword(buffer)) {
                    token_create(new_token, buffer, NULL, KEYWORD);
                } else {
                    token_create(new_token, buffer, NULL, IDENTIFIER);
                }
                //ungetc(c, file); // push back the non-identifier char
                
                return new_token;
            }
            break;

        case IN_BUILT_FUNC:
            if (isalnum(c) || c == '_') {
                c = fgetc(file); // actually consume it
                buffer_append(buffer, &pos, c, file, new_token);
            } else {
                token_create(new_token, buffer, NULL, IN_BUILT_FUNC);
                if (c != EOF) ungetc(c, file);
                buffer[pos] = '\0';
                return new_token;
            }
            break;


        default:
            break;
        }
    }
    token_create(new_token, NULL, NULL, -1); // EOF token
    return new_token;
}