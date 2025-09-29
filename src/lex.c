//////////////////////////////////////////////
// filename: lex.c	                   	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
// * Jozef Matus (xmatusj00) / karfisk 	    //
// * Jan Hajek (xhajekj00) / Wekk 	        //
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

/**
 * @brief Skips whitespace characters in the input file.
 * @param file Pointer to the file to read from
 * @param c The current character read from the file
 * @return The next non-whitespace character, or EOF if the end of the file
 *
 * this function is for consuming whitespace characters
 * you need to call ungetc() if you are planning to just peek
*/
int skip_whitespace(FILE *file, int c) {
	while (c != EOF && isblank(c)) {
		c = fgetc(file);
	}
	return c; // return the non-whitespace (or EOF)
}

static const struct KeywordEntry keyword_table[] = {
	{"class", KW_CLASS},
	{"if", KW_IF},
	{"else", KW_ELSE},
	{"is", KW_IS},
	{"null", KW_NULL},
	{"return", KW_RETURN},
	{"var", KW_VAR},
	{"import", KW_IMPORT},
	{"for", KW_FOR},
	{"for", KW_FOR},
	{"while", KW_WHILE},
	{"Ifj", KW_IFJ},
	{"static", KW_STATIC},
	{"true", KW_TRUE},
	{"false", KW_FALSE},
	{"Num", KW_NUM},
	{"String", KW_STRING},
	{"Null", KW_NULL_TYPE}
};

static const size_t keyword_count = sizeof(keyword_table) / sizeof(keyword_table[0]);

/**
 * @brief Checks if the token is a keyword and updates its type accordingly.
 * @param token Pointer to the token to check.
 */
void check_keyword(TokenPtr token) {
	for (size_t i = 0; i < keyword_count; i++) {
		if (strcmp(token->id, keyword_table[i].word) == 0) {
			token->type = keyword_table[i].type;
			if (token->type == KW_NULL) {
				token_update(token, NULL, "null", KW_NULL);
			}
			return; // stop after first match
		}
	}
	// If not found, token->type stays IDENTIFIER
}

// Helper function to safely append a character to the buffer and increment the position
static void buffer_append(char *buffer, size_t *pos, int c, FILE *file) {
	if (*pos >= MAX_BUFFER_LENGTH - 1) {
		program_error(file, ERR_INTERNAL, 2, NULL);
	}
	buffer[*pos] = c;
	(*pos)++;
	buffer[*pos] = '\0'; // Null-terminate the buffer
}

static void buffer_append_str(char *buffer, size_t *pos,
                              const char *str, FILE *file) {
    while (*str) {
        buffer_append(buffer, pos, *str++, file);
    }
}

// Save the last token when EOF is reached
static void save_penultimate_token(TokenPtr new_token, char *buffer, size_t pos) {
	// DOES NOT ACCOUNT FOR DATA YET
	if (pos != 0 && new_token->type != FILE_END) {
		switch (new_token->type) {
		case IDENTIFIER:
			if (pos > 1 && buffer[0]=='_' && buffer[1]=='_') {
					token_update(new_token, buffer, NULL, ID_GLOBAL_VAR);
					return;
				}

			token_update(new_token, buffer, NULL, IDENTIFIER);
			check_keyword(new_token);
			break;

		case NUMERICAL:
			token_update(new_token, NULL, buffer, new_token->type);
			break;

		default:
			break;
		}
	} else {
		token_update(new_token, NULL, NULL, FILE_END);
	}
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
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0; // buffer index

	c = fgetc(file);

	while(c != EOF) {
		switch (state)
		{
		case (START):

			if (c == '\n') {
				state = NEWLINE;
			}
			else if (isblank(c)) {
				c = fgetc(file);
				continue;
			}
			else if (strchr("(){},.", c)) { // for ternary expansion, then add '?' and ':' here too
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
				state = STRING; // STRING or MULTI_LINE_STRING
			}
			else if (isalpha(c) || c == '_') {
				state = IDENTIFIER;
			}
			else if (isdigit(c)) {
				state = NUMERICAL;
			}
			else {
				program_error(file, ERR_LEX, 0, NULL);
			}

			new_token->type = state;
			break;
		
		case NEWLINE: {
			do {
				c = fgetc(file);
			} while ( c == '\n');
			ungetc(c, file); // push back the first non-newline char
			token_update(new_token, "\\n", NULL, NEWLINE);
			return new_token;
		} // end NEWLINE
		
		case IDENTIFIER: {
			if (isalnum(c) || c == '_') {
				buffer_append(buffer, &pos, c, file); // build identifier
				c = fgetc(file);

			} else {
				ungetc(c, file);

				if (pos > 1 && buffer[0]=='_' && buffer[1]=='_') {
					token_update(new_token, buffer, NULL, ID_GLOBAL_VAR);
					return new_token;
				}

				token_update(new_token, buffer, NULL, IDENTIFIER);
				check_keyword(new_token);

				return new_token;
			}
			break;
		} // end IDENTIFIER


		case STRING: {
			c = fgetc(file);
			if (c == '\\') { // special char
				state = STRING_SPECIAL;
			}
			else if (c == '\"') { // end of string
				token_update(new_token, NULL, buffer, STRING);
				return new_token;
			}
			else if (c == '\n') {
				token_update(new_token, NULL, buffer, STRING);
				program_error(file, ERR_LEX, 1, new_token); // unterminated string
			}
			else
				buffer_append(buffer, &pos, c, file); // build string

			break;
		} // end STRING

		case STRING_SPECIAL: {
			c = fgetc(file);

			if (c == EOF || c == '\n') {
				token_update(new_token, NULL, buffer, STRING);
				program_error(file, ERR_LEX, 1, new_token);
			}

			switch (c) {
				case 'n':
					buffer_append(buffer, &pos, '\n', file);
					break;
				case 't':
					buffer_append(buffer, &pos, '\t', file);
					break;
				case 'r':
					buffer_append(buffer, &pos, '\r', file);
					break;
				case '"':
					buffer_append(buffer, &pos, '\"', file);
					break;
				case '\\':
					buffer_append(buffer, &pos, '\\', file);
					break;
				case 'x': {
					char hex[3];
					hex[0] = fgetc(file);
					hex[1] = fgetc(file);
					hex[2] = '\0';

					if (!isxdigit(hex[0]) || !isxdigit(hex[1])) {
						token_update(new_token, NULL, buffer, STRING);
						program_error(file, ERR_LEX, 3, new_token); // invalid hex escape
					}
					unsigned char val = (unsigned char) strtol(hex, NULL, 16); // convert hex to char
					buffer_append(buffer, &pos, val, file);
					break;
				}
				default:
					token_update(new_token, NULL, buffer, STRING);
					program_error(file, ERR_LEX, 2, new_token); // invalid escape
			}

			state = STRING;
			break;
		} // end STRING_SPECIAL

		case ARITHMETICAL: {
			int temp = c;        // operator candidate
			c = fgetc(file);     // lookahead

			if (temp == '/') {
				if (c == '/') {
					state = COMMENT;
					ungetc(c, file); // put back the second '/'
					break;
				} else if (c == '*') {
					state = BLOCK_COMMENT; // NOT IMPLEMENTED YET
					break;
				} else {
					ungetc(c, file);
					buffer_append(buffer, &pos, temp, file);
					token_update(new_token, buffer, NULL, ARITHMETICAL);
					return new_token;
				}
			}

			// For +, -, *
			ungetc(c, file);
			buffer_append(buffer, &pos, temp, file);
			token_update(new_token, buffer, NULL, ARITHMETICAL);
			return new_token;
		} // end ARITHMETICAL

		case COMMENT: { 
			c = fgetc(file);
			token_update(new_token, "\\n", NULL, NEWLINE); // update in advance (possible EOF)
			if (c == '\n') {
				state = NEWLINE; // single-line comment = newline token
			}
			break;
		} // end COMMENT

		case SPECIAL: {
			buffer_append(buffer, &pos, c, file);
			token_update(new_token, buffer, NULL, SPECIAL);
			return new_token;
			break;
		} // end SPECIAL

		case CMP_OPERATOR: {
			buffer_append(buffer, &pos, c, file); // first char (<, >, =, or !)
			c = fgetc(file);

			if (buffer[0] == '=') {
				if (c == '=') { // only "=="
					buffer_append(buffer, &pos, c, file);
				} else {
					ungetc(c, file); // single "=" is assignment
					token_update(new_token, buffer, NULL, SPECIAL); 
					return new_token;
				}
			}
			else if (buffer[0] == '<' || buffer[0] == '>') {
				if (c == '=') { // <= or >=
					buffer_append(buffer, &pos, c, file);
				} else {
					ungetc(c, file); // just < or >
				}
			}

			token_update(new_token, buffer, NULL, CMP_OPERATOR);
			return new_token;
		} // end CMP_OPERATOR
		
		case NOT_EQUAL: {
			c = fgetc(file);
			
			if (c == '=') {
				token_update(new_token, "!=", NULL, NOT_EQUAL);
				return new_token;
			}
			buffer_append(buffer, &pos, '!', file);
			buffer_append(buffer, &pos, c, file);
			token_update(new_token, buffer, NULL, NOT_EQUAL);
			program_error(file, ERR_LEX, 4, new_token); // invalid usage: '!' must be followed by '='
			
			break;
		} // end NOT_EQUAL

		case NUMERICAL: { // NOT FULLY IMPLEMENTED YET
			if (isdigit(c)) {
				buffer_append(buffer, &pos, c, file);
				// stay in NUMERICAL, next iteration of outer while will grab next digit
			} else {
				ungetc(c, file);  // put back the non-digit
				token_update(new_token, NULL, buffer, NUMERICAL);
				return new_token;
			}
			c = fgetc(file);
			break;
		} // end NUMERICAL

		default:
			program_error(file, ERR_INTERNAL, 1, new_token); // unknown token
			break;
		}
	} // while(!EOF)


	save_penultimate_token(new_token, buffer, pos);
	(void)buffer_append_str; // REMOVE , JUST SO COMPILER DOESNT Complain
	return new_token;
}