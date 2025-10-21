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
#include <regex.h>

/**
 * @file lex.c
 * @brief generates a single token from the open file
 *
 * @note Implements a finite state machine (FSM) to tokenize the input
 */

#define FINALIZE_NUM() do { \
    ungetc(c, file); \
    token_update(token, NULL, buffer, NUMERICAL); \
} while (0)

#define APPEND_TO_BUFFER() do { \
	buffer_append(buffer, &pos, c, file); \
} while (0)

#define LEX_INVAL_TOK_ERR() do { \
	program_error(file, ERR_LEX, ERR_MSG_INVALID_TOK, token); \
} while (0)

// calculate the number of keywords in the table
static const size_t keyword_count = sizeof(keyword_table) / sizeof(keyword_table[0]);

typedef enum {
    NUM_START,
	NUM_ZERO,
    NUM_DEC,
	NUM_HEX_START,
    NUM_HEX,
	NUM_FRAC_START,
    NUM_FRAC,
    NUM_EXP_START,
    NUM_EXP_SIGN,
    NUM_EXP,
} NumState;

typedef enum {
	NC_seek,
	NC_star,
	NC_slash
} NC_state;

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
			break; // stop after first match
		}
	}
	return;
}

/**
 * @brief Skips whitespace characters in the input file.
 * @param file Pointer to the file to read from
 * @param c The current character read from the file
 * @return The next non-whitespace character, or EOF if the end of the file
*/
int skip_whitespace(FILE *file, int c) {
	while (c != EOF && isblank(c)) {
		c = fgetc(file);
	}
	return c; // return the non-whitespace (or EOF)
}

// Helper function to safely append a character to the buffer and increment the position
void buffer_append(char *buffer, size_t *pos, int c, FILE *file) {
	if (*pos >= MAX_BUFFER_LENGTH - 1) {
		program_error(file, ERR_INTERNAL, ERR_MSG_BUFF_OVERFLOW, NULL);
	}
	buffer[*pos] = c;
	(*pos)++;
	buffer[*pos] = '\0'; // Null-terminate the buffer
}

/**
 * @brief Function implementing FSM for creating Num type
 * @return TokenPtr num_token
 */
int numerizer (TokenPtr token, int c, FILE* file) {
	NumState state = NUM_START;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0;

	while (c != EOF) {
		switch (state) {
		case NUM_START: {
			if (c == '0') {
				APPEND_TO_BUFFER();
				state = NUM_ZERO;
				break;
			} else if (isdigit(c)) {
				APPEND_TO_BUFFER();
				state = NUM_DEC;
				break;
			} 
			return 0; // error -- first input wasn't a digit
		} // end NUM_START
		case NUM_ZERO: {
			c = fgetc(file);
			if ((c = tolower(c)) == 'x') {
				APPEND_TO_BUFFER();
				state = NUM_HEX;
				break;
			} else if (c == '.') {
				APPEND_TO_BUFFER();
				state = NUM_FRAC_START;
				break;
			} else if ((c = tolower(c)) == 'e') {
				APPEND_TO_BUFFER();
				state = NUM_EXP_SIGN;
				break;
			} else if (isblank(c) || c == EOF) {
				FINALIZE_NUM();
				return 1; // standalone '0'
			} 
			return 0; // leading 0 forbidden
		}
		case NUM_DEC: {
			c = fgetc(file);
			if (isdigit(c)) {
				// continue building
				APPEND_TO_BUFFER();
				break;
			} else if (c == '.') {
				// go to fraction
				APPEND_TO_BUFFER();
				state = NUM_FRAC_START;
				break;
			} else if ((c = tolower(c)) == 'e') {
				// go to exponent
				APPEND_TO_BUFFER();
				state = NUM_EXP_SIGN;
				break;
			} else if (isspace(c) || c == EOF) {
				FINALIZE_NUM();
				return 1; // done
			}
			return 0; // error
		
		} // end NUM_DEC
		case NUM_FRAC_START: {
			// ensures . is present
			c = fgetc(file);
			if (isdigit(c)) {
				APPEND_TO_BUFFER();
				state = NUM_FRAC;
				break;
			}
			return 0; // error
		} // end NUM_FRAC_START
		case NUM_FRAC: {
			c = fgetc(file);

			if (isdigit(c)) {
				// continue building
				APPEND_TO_BUFFER();
			}
			else if ((c = tolower(c)) == 'e') {
				// go to exponent
				APPEND_TO_BUFFER();
				state = NUM_EXP_SIGN;
			}
			else {
				FINALIZE_NUM();
				return 1; // done
			}
			break;
		} // end NUM_FRAC
		case NUM_EXP_SIGN: {
			c = fgetc(file);

			if (c == '-' || c == '+') {
				// go to decimal part of exp
				APPEND_TO_BUFFER();
				state = NUM_EXP_START;
				break;
			} else if (isdigit(c)) {
				APPEND_TO_BUFFER();
				state = NUM_EXP;
				break;
			}
			return 0; // error
		} // end NUM_EXP_SIGN
		case NUM_EXP_START: {
			c = fgetc(file);

			if (!isdigit(c))
				return 0; // error
			
			APPEND_TO_BUFFER();
			state = NUM_EXP;
			break;
		} // end NUM_EXP_START
		case NUM_EXP: {
			c = fgetc(file);

			if (isdigit(c)) {
				// comtinue building
				APPEND_TO_BUFFER();
			}
			else {
				FINALIZE_NUM();
				return 1; // done
			}
			break;
		} // end NUM_EXP
		case NUM_HEX_START: {
			// ensures 0x is present
			c = fgetc(file);
			if (isxdigit(c)) {
				APPEND_TO_BUFFER();
				state = NUM_HEX;
				break;
			}
			return 0; // error
		} // end NUM_HEX_START
		case NUM_HEX: {
			c = fgetc(file);
			if(isxdigit(c)) {
				APPEND_TO_BUFFER();
			} else {
				FINALIZE_NUM();
				return 1;
			}
			break;
		} // end NUM_HEX
		} // switch
	} // loop until EOF
	return 0; // unexpected end
}

/**
 * @brief Function implementing FSM to check validity of (nested) block comments
 * @return 0 on fail, otherwise 1
 * @note Assumes sequence of `/`, `*` has been reached, e.g. using fgetc(),  next char 
 * would be the comments body.. until final `*`,`/`
 */
int no_comment (FILE *file) {
	int c = 0;
	NC_state state = NC_seek;
	size_t nest_lvl = 1; // inside a comment already

	while (c != EOF) {
		switch(state) {
			case NC_seek: {
				// comment finished
				if (nest_lvl == 0) return 1;

				c = fgetc(file);
				if (c == '*') state = NC_star;
				else if (c == '/') state = NC_slash;
				break;
			}
			case NC_star: {
				// can decrease nest level
				c = fgetc(file);
				if (c == '/') {
					nest_lvl--;
				}
				state = NC_seek;
				break;
			}
			case NC_slash: {
				// can increase nest level
				c = fgetc(file);
				if (c == '*') nest_lvl++;
				state = NC_seek;
				break;
			}
			default:
				break;
		} // end switch
	}
	return 0;
}

/**
 * @brief Scans the input file and generates tokens.
 * @param file Pointer to the file to scan.
 * @return
 * - `-1` in token->type if the end of the file was reached.
 * - `program_error` on lexical or internal errors.
 *
 * @note The caller is responsible for handling the returned tokens and managing
 *       memory associated with the token structure.
 *       Ensure that the file pointer is valid and opened in read mode before calling lexer()
 */
TokenPtr lexer(FILE *file) {

	TokenPtr token = token_init();

	int state = START;
	int c;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0; // buffer index

	c = fgetc(file);

	while(c != EOF) {
		switch (state)
		{
		case (START): {
			if (c == '\n') {
				state = NEWLINE;
			}
			else if (isblank(c)) {
				c = fgetc(file);
				continue;
			}
			else if (strchr("(){},.", c)) { // for ternary operator,  add '?', ':'
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
				APPEND_TO_BUFFER(); // start building identifier
			}
			else if (isdigit(c)) {
				state = NUMERICAL;
			}
			else {
				APPEND_TO_BUFFER();
				token_update(token, buffer, NULL, -2);
				program_error(file, ERR_LEX, ERR_MSG_NOT_IMPLEMENTED, token);
			}
			token->type = state;
			break;
		} // end START

		case NEWLINE: {
			do {
				c = fgetc(file);
			} while ( isspace(c)); // eat all whitespaces, newlines
			if (c == '/') {
				c = fgetc(file);
				if (c == '*') {
					state = BLOCK_COMMENT;
					break;
				}
				else if (c == '/') {
					state = COMMENT;
					break;
				}
			}
			ungetc(c, file); // push back non-newline char
			token_update(token, "\\n", NULL, NEWLINE);
			return token;
		} // end NEWLINE

		case IDENTIFIER: {
			c = fgetc(file);

			if (isalnum(c) || c == '_') {
				APPEND_TO_BUFFER(); // build identifier
			} else {
				// char on the edge of the identif could be new valid tok e.g. a+b
				if(!isspace(c) && c != EOF &&!strchr("(){},.<=>/+-*!/", c)) 
					LEX_INVAL_TOK_ERR();
				ungetc(c, file);
				// "_" invalid by itself
				if (pos == 1 && buffer[0] == '_') { 
					LEX_INVAL_TOK_ERR(); 
				}
				// GLOBAL ID
				else if (pos > 1 && buffer[0]=='_' && buffer[1]=='_') {
					// must not start with digit
					if (!isalpha(buffer[2]) && buffer[2] != '\0' ) 
						LEX_INVAL_TOK_ERR();
					token_update(token, buffer, NULL, ID_GLOBAL_VAR);
					return token;
				}
				token_update(token, buffer, NULL, IDENTIFIER);
				check_keyword(token); // may promote identif to kw

				return token;
			}
			break;
		} // end IDENTIFIER

		case STRING: {
			c = fgetc(file);
			if (c == '\\') { // special char
				state = STRING_SPECIAL;
			}
			else if (c == '\"') { // end of string
				token_update(token, NULL, buffer, STRING);
				return token;
			}
			else if (c == '\n') {
				token_update(token, NULL, buffer, STRING);
				LEX_INVAL_TOK_ERR(); // unterminated string
			}
			else
				APPEND_TO_BUFFER(); // build string

			break;
		} // end STRING

		case STRING_SPECIAL: {
			c = fgetc(file);

			if (c == EOF || c == '\n') {
				token_update(token, NULL, buffer, STRING);
				LEX_INVAL_TOK_ERR();
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
					token_update(token, NULL, buffer, STRING);
					LEX_INVAL_TOK_ERR(); // invalid hex escape
				}
				unsigned char val = (unsigned char) strtol(hex, NULL, 16); // convert hex to char
				buffer_append(buffer, &pos, val, file);
				break;
			}
			default:
				token_update(token, NULL, buffer, STRING);
				LEX_INVAL_TOK_ERR(); // invalid escape
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
					break;
				} else if (c == '*') {
					state = BLOCK_COMMENT;
					break;
				} else {
					ungetc(c, file);
					buffer_append(buffer, &pos, temp, file);
					token_update(token, buffer, NULL, ARITHMETICAL);
					return token;
				}
			}

			// For +, -, *
			ungetc(c, file); // push back lookahead char
			buffer_append(buffer, &pos, temp, file);
			token_update(token, buffer, NULL, ARITHMETICAL);
			return token;
		} // end ARITHMETICAL

		case COMMENT: {
			c = fgetc(file);
			token_update(token, "\\n", NULL, NEWLINE); // line comment is considered a newline
			if (c == '\n') {
				state = NEWLINE; // single-line comment = newline token
			}
			break;
		} // end COMMENT

		case BLOCK_COMMENT: { // BLOCK COMMENTS can be nested
			if (!no_comment(file)) { 
				token_update(token, NULL, NULL, BLOCK_COMMENT);
				program_error(file, ERR_MSG_UNENCLOSED_COMM, ERR_LEX, token);
			}
			c = fgetc(file);
			state = START; // continue reading
			break;
		}

		case SPECIAL: {
			APPEND_TO_BUFFER();
			token_update(token, buffer, NULL, SPECIAL);
			// bad float check, e.g. ".123"
			if (c == '.') { 
				c = fgetc(file); 
				if (isdigit(c)) {
					LEX_INVAL_TOK_ERR();
				}
				ungetc(c, file);
			}
			return token;
		} // end SPECIAL

		case CMP_OPERATOR: {
			APPEND_TO_BUFFER(); // first char (<, >, =, or !)
			c = fgetc(file);

			if (buffer[0] == '=') {
				if (c == '=') { // only "=="
					APPEND_TO_BUFFER();
				} else {
					ungetc(c, file); // single "=" is assignment
					token_update(token, buffer, NULL, SPECIAL);
					return token;
				}
			}
			else if (buffer[0] == '<' || buffer[0] == '>') {
				if (c == '=') { // <= or >=
					APPEND_TO_BUFFER();
				} else {
					ungetc(c, file); // just < or >
				}
			}

			token_update(token, buffer, NULL, CMP_OPERATOR);
			return token;
		} // end CMP_OPERATOR

		case NOT_EQUAL: {
			c = fgetc(file);

			if (c == '=') {
				token_update(token, "!=", NULL, NOT_EQUAL);
				return token;
			}
			LEX_INVAL_TOK_ERR(); // invalid usage: '!' must be followed by '='
			break;
		} // end NOT_EQUAL

		case NUMERICAL: {
			token_update(token, NULL, NULL, NUMERICAL);
			if (numerizer(token, c, file)) {
				return token;
			} else {
				LEX_INVAL_TOK_ERR();
			}
			break;
		}

		default:
			program_error(file, ERR_MSG_NOT_IMPLEMENTED, ERR_LEX, token); // unknown token
		}
	} // while(!EOF)
	return token;
}