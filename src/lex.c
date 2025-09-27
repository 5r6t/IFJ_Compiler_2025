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
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0; // buffer index

	c = fgetc(file);

	while(c != EOF) {
		switch (state)
		{
		// ignore whitespace for now
		case (START):

			if (c == '\n') {
				state = NEWLINE;
			}
			else if (isblank(c)) {
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
				state = STRING; // STRING or MULTI_LINE_STRING
			}
			else if (isalpha(c) || c == '_') {
				state = IDENTIFIER;
			}
			else if (isdigit(c)) {
				state = OUR_INT; // can become float later
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
		
		case IDENTIFIER: { /// FUUUUCK
			if (isalnum(c) || c == '_') {
				buffer_append(buffer, &pos, c, file, new_token); // build identifier
				c = fgetc(file);

			} else {
				c = skip_whitespace(file, c); // peek next non-blank for IN_BUILT_FUNC
				if (c == '.') { // IDENTIF/KEYWORD "."  IDENTIF
					buffer_append(buffer, &pos, c, file, new_token);
					state = IN_BUILT_FUNC;
					c = fgetc(file); //consume the dot
					
					// ?? I AM SORRY FOR THIS ??
					// but I can't cram 3tokens into one otherwise
					if (isblank(c)) skip_whitespace(file, c);
					else if (c == '\n') { 
						// I'll allow one newline, just one idc
						c = fgetc(file);
					}
					if(isblank(c)) c = skip_whitespace(file, c); // check again, sob

					break; // go build second part of IDENTIF
				}

				buffer[pos] = '\0';
				ungetc(c, file);

				if (pos > 1 && buffer[0]=='_' && buffer[1]=='_') {
					token_update(new_token, buffer, NULL, ID_GLOBAL_VAR);
					return new_token;
				}

				token_update(new_token, buffer, NULL, IDENTIFIER); //!!!!

				check_keyword(new_token);

				return new_token;
			}
			break;
		} // end IDENTIFIER

		case IN_BUILT_FUNC: {

			static bool one_n = false;
			if ( one_n == false && c == '\n') {
				one_n = true;
				continue;
			}
			if (isalnum(c) || c == '_') {
				buffer_append(buffer, &pos, c, file, new_token);
				c = fgetc(file); // consume char

			} else {
				buffer[pos] = '\0'; //
				token_update(new_token, buffer, NULL, IN_BUILT_FUNC);
				return new_token;
			}
			break;
		} // end IN_BUILT_FUNC

		case STRING: {
			c = fgetc(file);
			if (c == '\\') { // special char
				state = STRING_SPECIAL;
			}
			else if (c == '\"') { // end of string
				buffer[pos] = '\0';
				token_update(new_token, NULL, buffer, STRING);
				return new_token;
			}
			else if (c == '\n') {
				buffer[pos] = '\0';
				token_update(new_token, NULL, buffer, STRING);
				program_error(file, ERR_LEX, 1, new_token); // unterminated string
			}
			else
				buffer_append(buffer, &pos, c, file, new_token); // build string

			break;
		} // end STRING

		case STRING_SPECIAL: {
			c = fgetc(file);

			if (c == EOF || c == '\n') {
				buffer[pos] = '\0';
				token_update(new_token, NULL, buffer, STRING);
				program_error(file, ERR_LEX, 1, new_token);
			}

			switch (c) {
				case 'n':
					buffer_append(buffer, &pos, '\n', file, new_token);
					break;
				case 't':
					buffer_append(buffer, &pos, '\t', file, new_token);
					break;
				case 'r':
					buffer_append(buffer, &pos, '\r', file, new_token);
					break;
				case '"':
					buffer_append(buffer, &pos, '\"', file, new_token);
					break;
				case '\\':
					buffer_append(buffer, &pos, '\\', file, new_token);
					break;
				case 'x': {
					char hex[3];
					hex[0] = fgetc(file);
					hex[1] = fgetc(file);
					hex[2] = '\0';

					if (!isxdigit(hex[0]) || !isxdigit(hex[1])) {
						buffer[pos] = '\0';
						token_update(new_token, NULL, buffer, STRING);
						program_error(file, ERR_LEX, 3, new_token); // invalid hex escape
					}
					unsigned char val = (unsigned char) strtol(hex, NULL, 16); // convert hex to char
					buffer_append(buffer, &pos, val, file, new_token);
					break;
				}
				default:
					buffer[pos] = '\0';
					token_update(new_token, NULL, buffer, STRING);
					program_error(file, ERR_LEX, 2, new_token); // invalid escape
			}

			state = STRING;
			break;
		} // end STRING_SPECIAL

		case ARITHMETICAL: {
			int temp = c;
			c = fgetc(file);

			if (temp == '/' && c == '/') { // not a division, a comment // IDENTIFIER FUCKING IT UP
				state = COMMENT;
				break;
			}
			else if (temp == '/' && c == '*') {

			}
			break;
		} // end ARITHMETICAL

		case COMMENT: { // single-line comment = newline token
			c = fgetc(file);
			if (c == '\n') {
				state = NEWLINE;
			}
			break;
		} // end COMMENT

		case -2590: {
			break;
		} // end MULTILINE_COMMENT

		default:
			DEBUG_PRINT("UNKNOWN REEEEEEEEEe");
			program_error(file, ERR_INTERNAL, 1, new_token);
			break;
		}
	} // while(!EOF)

	// ??? THIS DOES NOT ACCOUNT FOR DATA YET -> DATA BUFFER -> #NEXT_MEETING -> do we even need data??
	if (new_token->type != FILE_END) //  has a type => definitely a non EOF token
	{
		if (pos != 0) {
			buffer[pos] = '\0';
			token_update(new_token, buffer, new_token->data, new_token->type);
		}
	}
	else
		token_update (new_token, NULL, NULL, FILE_END);


	return new_token;
}