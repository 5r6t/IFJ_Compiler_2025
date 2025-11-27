//////////////////////////////////////////////
// filename: lex.c	                   	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
//////////////////////////////////////////////

#include "common.h"
#include "lex.h"

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

/// @brief abstracts from returning and updating token
#define FINALIZE_TOK_ID(type)                      \
	do                                             \
	{                                              \
		ungetc(c, file);                           \
		token_update(token, buffer, NULL, (type)); \
	} while (0)

#define FINALIZE_TOK_DATA(type)                    \
	do                                             \
	{                                              \
		ungetc(c, file);                           \
		token_update(token, NULL, buffer, (type)); \
	} while (0)

/// @brief abstracts from typing parameters except input
#define APPEND_TO_BUFFER(c)                     \
	do                                          \
	{                                           \
		buffer_append(buffer, &pos, (c), file); \
	} while (0)

/// @brief strongly dependent on names, avoid changing
#define FLUSH_APPEND_TO_BUFF(c)                               \
	do                                                        \
	{                                                         \
		buffer_flush_to(buffer, &pos, tmp_buff, &tpos, file); \
		buffer_append(buffer, &pos, (c), file);               \
	} while (0)

#define LEX_INVAL_TOK_ERR()                                       \
	do                                                            \
	{                                                             \
		program_error(file, ERR_LEX, ERR_MSG_INVALID_TOK, token); \
	} while (0)

// calculate the number of keywords in the table
static const size_t keyword_count = sizeof(keyword_table) / sizeof(keyword_table[0]);

typedef enum
{
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

typedef enum
{
	NC_seek,
	NC_star,
	NC_slash
} NC_state;

typedef enum
{
	CMP_start,
	CMP_assign,
	CMP_gl,
	CMP_neq,
	CMP_end_w_eq
} CMP_state;

/**
 * @brief Checks if the token is a keyword and updates its type accordingly.
 * @param token Pointer to the token to check.
 */
void check_keyword(TokenPtr token)
{
	for (size_t i = 0; i < keyword_count; i++)
	{
		if (strcmp(token->id, keyword_table[i].word) == 0)
		{
			token->type = keyword_table[i].type;
			if (token->type == KW_NULL)
			{
				token_update(token, NULL, "null", KW_NULL);
			} // null is value
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
int skip_whitespace(FILE *file, int c)
{
	while (c != EOF && isblank(c))
	{
		c = fgetc(file);
	}
	return c; // return the non-whitespace (or EOF)
}

/// @brief Helper to safely append a character to the buffer and increment the position
void buffer_append(char *buffer, size_t *pos, int c, FILE *file)
{
	if (*pos >= MAX_BUFFER_LENGTH - 1)
		program_error(file, ERR_INTERNAL, ERR_MSG_BUFF_OVERFLOW, NULL);

	buffer[*pos] = c;
	(*pos)++;
	buffer[*pos] = '\0'; // Null-terminate the buffer
}

/// @brief Helper to safely append a string to the buffer and increment the position
/// @note flushing src to dest, can handle s_pos being NULL, resets source!
void buffer_flush_to(char *dest, size_t *d_pos, char *src, size_t *s_pos, FILE *file)
{
	for (size_t i = 0; src[i] != '\0'; i++)
	{
		if (*d_pos >= MAX_BUFFER_LENGTH - 1)
			program_error(file, ERR_INTERNAL, ERR_MSG_BUFF_OVERFLOW, NULL);
		dest[(*d_pos)++] = src[i];
	}
	dest[*d_pos] = '\0';

	// reset source
	if (s_pos)
		*s_pos = 0;
	src[0] = '\0'; // assuming only buffer_append will be used to fill the buffer next time
}

/// @brief Helper to convert char into escape char
int decode_esc(int c)
{
	const char *from = "ntr\"\\";
	const char *to = "\n\t\r\"\\";
	const char *p = strchr(from, c);
	return p ? to[p - from] : c;
}

/**
 * @brief Function implementing FSM, creates Num type
 * @return 0 on fail, otherwise 1
 */
int numerizer(TokenPtr token, int c, FILE *file)
{
	NumState state = NUM_START;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0;

	while (c != EOF)
	{
		switch (state)
		{
		case NUM_START:
		{
			APPEND_TO_BUFFER(c);
			if (c == '0')
			{
				state = NUM_ZERO;
				break;
			}
			else if (isdigit(c))
			{
				state = NUM_DEC;
				break;
			}
			return 0; // error -- first input wasn't a digit
		} // end NUM_START
		case NUM_ZERO:
		{
			c = fgetc(file);
			if ((c = tolower(c)) == 'x')
			{
				APPEND_TO_BUFFER(c);
				state = NUM_HEX_START;
				break;
			}
			else if (c == '.')
			{
				APPEND_TO_BUFFER(c);
				state = NUM_FRAC_START;
				break;
			}
			else if ((c = tolower(c)) == 'e')
			{
				APPEND_TO_BUFFER(c);
				state = NUM_EXP_SIGN;
				break;
			}
			else if (c == '0')
			{
				return 0; // err leading zero
			}
			else
			{
				FINALIZE_TOK_DATA(NUMERICAL);
				return 1; // standalone '0'
			}
		}
		case NUM_DEC:
		{
			c = fgetc(file);
			if (isdigit(c))
			{
				// continue building
				APPEND_TO_BUFFER(c);
				break;
			}
			else if (c == '.')
			{
				// go to fraction
				APPEND_TO_BUFFER(c);
				state = NUM_FRAC_START;
				break;
			}
			else if ((c = tolower(c)) == 'e')
			{
				// go to exponent
				APPEND_TO_BUFFER(c);
				state = NUM_EXP_SIGN;
				break;
			}
			else
			{
				FINALIZE_TOK_DATA(NUMERICAL);
				return 1; // done
			}

		} // end NUM_DEC
		case NUM_FRAC_START:
		{
			// ensures . is present
			c = fgetc(file);
			if (isdigit(c))
			{
				APPEND_TO_BUFFER(c);
				state = NUM_FRAC;
				break;
			}
			return 0; // error
		} // end NUM_FRAC_START
		case NUM_FRAC:
		{
			c = fgetc(file);
			if (isdigit(c))
			{
				// continue building
				APPEND_TO_BUFFER(c);
			}
			else if ((c = tolower(c)) == 'e')
			{
				// go to exponent
				APPEND_TO_BUFFER(c);
				state = NUM_EXP_SIGN;
			}
			else
			{
				FINALIZE_TOK_DATA(NUMERICAL);
				return 1; // done
			}
			break;
		} // end NUM_FRAC
		case NUM_EXP_SIGN:
		{
			c = fgetc(file);
			if (c == '-' || c == '+')
			{
				// go to decimal part of exp
				APPEND_TO_BUFFER(c);
				state = NUM_EXP_START;
				break;
			}
			else if (isdigit(c))
			{
				APPEND_TO_BUFFER(c);
				state = NUM_EXP;
				break;
			}
			return 0; // error
		} // end NUM_EXP_SIGN
		case NUM_EXP_START:
		{
			c = fgetc(file);
			if (!isdigit(c))
				return 0; // error

			APPEND_TO_BUFFER(c);
			state = NUM_EXP;
			break;
		} // end NUM_EXP_START
		case NUM_EXP:
		{
			c = fgetc(file);
			if (isdigit(c))
			{
				APPEND_TO_BUFFER(c); // continue building
			}
			else
			{
				FINALIZE_TOK_DATA(NUMERICAL);
				return 1; // done
			}
			break;
		} // end NUM_EXP
		case NUM_HEX_START:
		{
			// ensures 0x is present
			c = fgetc(file);
			if (isxdigit(c))
			{
				APPEND_TO_BUFFER(c);
				state = NUM_HEX;
				break;
			}
			return 0; // error
		} // end NUM_HEX_START
		case NUM_HEX:
		{
			c = fgetc(file);
			if (isxdigit(c))
			{
				APPEND_TO_BUFFER(c);
			}
			else
			{

				FINALIZE_TOK_DATA(NUMERICAL);
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
 * @note Assumes sequence of `/`,`*` has been reached, next char
 * being the comments body.. until final `*`,`/`
 */
int no_comment(FILE *file)
{
	int c = 0;
	NC_state state = NC_seek;
	size_t nest_lvl = 1; // inside a comment already /, *

	while (c != EOF)
	{
		switch (state)
		{
		case NC_seek:
		{
			// comment finished
			if (nest_lvl == 0)
				return 1;

			c = fgetc(file);
			if (c == '*')
				state = NC_star;
			else if (c == '/')
				state = NC_slash;
			break;
		}
		case NC_star:
		{
			// can decrease nest level
			c = fgetc(file);
			if (c == '/')
			{
				nest_lvl--;
			}
			if (c != '*')
				state = NC_seek;
			break;
		}
		case NC_slash:
		{
			// can increase nest level
			c = fgetc(file);
			if (c == '*')
				nest_lvl++;
			state = NC_seek;
			break;
		}
		default:
			break;
		} // end switch
	} // end while
	return 0; // fallback
}

/**
 * @brief FSM to create cmp operators and assign tokens
 * @return 0 on fail, otherwise 1
 */
int get_cmp_op(TokenPtr token, int c, FILE *file)
{
	CMP_state state = CMP_start;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0;

	while (c != EOF)
	{
		switch (state)
		{
		case CMP_start:
		{
			if (c == '=')
				state = CMP_assign;
			else if (c == '!')
				state = CMP_neq;
			else if (strchr("<>", c))
				state = CMP_gl;
			else
				LEX_INVAL_TOK_ERR();
			APPEND_TO_BUFFER(c); // append <>!=
			break;
		} // end CMP_start

		case CMP_assign:
		{
			c = fgetc(file);
			if (c == '=')
			{ // ==
				state = CMP_end_w_eq;
			}
			else
			{ // single equals
				ungetc(c, file);
				token_update(token, buffer, NULL, SPECIAL);
				return 1;
			} 
			break;
		} // end CMP_equal_s
		case CMP_gl:
		{
			c = fgetc(file);
			// <= or >=
			if (c == '=')
				state = CMP_end_w_eq;
			else
			{ // single < or >
				FINALIZE_TOK_ID(CMP_OPERATOR);
				return 1;
			}
			break;
		} // end CMP_gl

		case CMP_neq:
		{
			c = fgetc(file);
			if (c != '=') LEX_INVAL_TOK_ERR();
			state = CMP_end_w_eq;
			break;
		} // end CMP_neq

		case CMP_end_w_eq:
		{
			APPEND_TO_BUFFER(c); // append '='
			token_update(token, buffer, NULL, CMP_OPERATOR);
			return 1;
		}
		default:
			break;
		} // end switch
	} // end while
	return 0; // fallback
}

/**
 * @brief FSM to create strings from strings/multiline strings
 * @return 0 on fail, otherwise 1
 */

typedef enum
{
	STR_start,
	STR_em_mt,	// empty or multiline
	STR_single, // like me
	STR_special,
	STR_multi,
	STR_multi_q,
	STR_multi_end,
	STR_count,
	STR_special_l, // letter
	STR_special_hex_s,
	STR_special_hex,
	STR_finish
} STR_state;

int get_string(TokenPtr token, int c, FILE *file)
{
	STR_state state = STR_start;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	char tmp_buff[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0;
	size_t tpos = 0;

	if (c != '\"')
		LEX_INVAL_TOK_ERR(); // have to enter with quote
	c = fgetc(file);
	while (c != EOF)
	{
		switch (state)
		{
		case STR_start:
		{
			if (c == '\"')
				state = STR_em_mt; // second quote
			else if (c == '\\')
				state = STR_special;
			else
			{
				state = STR_single;
				APPEND_TO_BUFFER(c);
			}

			break;
		} // end STR_start
		case STR_em_mt:
		{
			c = fgetc(file);
			if (c == '\"')
				state = STR_multi; // third quote
			else
			{ // empty string
				FINALIZE_TOK_DATA(STRING);
				return 1;
			}
			break;
		} // end STR_em_mt
		case STR_multi:
		{
			c = fgetc(file);
			if (isspace(c)) // store whitespaces
				buffer_append(tmp_buff, &tpos, c, file);
			else if (c == '\"')
			{
				state = STR_multi_q; // first quote
				buffer_append(tmp_buff, &tpos, c, file);
			}
			else
			{ // another character
				FLUSH_APPEND_TO_BUFF(c);
			}
			break;
		} // STR_multi
		case STR_multi_q:
		{
			c = fgetc(file);
			if (c == '\"')
			{ // end is near >w<
				state = STR_multi_end;
				buffer_append(tmp_buff, &tpos, c, file);
			}
			else
			{
				FLUSH_APPEND_TO_BUFF(c);
				state = STR_multi;
			}
			break;
		} // end STR_multi_q
		case STR_multi_end:
		{
			c = fgetc(file);
			if (c == '\"')
			{ // third quote
				FINALIZE_TOK_DATA(STRING);
				return 1; // not flushing tmp_buffer
			}
			else
			{
				FLUSH_APPEND_TO_BUFF(c);
				state = STR_multi;
			}
			break;
		} // end STR_multi_end
		case STR_single:
		{
			c = fgetc(file);
			if (c == '\\')
				state = STR_special;
			else if (c == '\"')
			{ // string end
				token_update(token, NULL, buffer, STRING);
				return 1;
			}
			else if (c == '\n')
			{
				LEX_INVAL_TOK_ERR();
			}
			else
			{
				if (iscntrl(c))
					LEX_INVAL_TOK_ERR(); // unescaped control char
				APPEND_TO_BUFFER(c);	 // continue building
			}
			break;
		} // end STR_SINGLE
		case STR_special:
		{
			c = fgetc(file);
			if (c == 'x')
				state = STR_special_hex_s;
			else if (strchr("ntr\"\\", c))
			{
				APPEND_TO_BUFFER(decode_esc(c));
				state = STR_single;
			}
			else
				LEX_INVAL_TOK_ERR();
			break;
		} // end STR_special
		case STR_special_hex_s:
		{
			char hex[3] = {'\0'};
			hex[0] = fgetc(file);
			hex[1] = fgetc(file);
			if (!isxdigit(hex[0]) || !isxdigit(hex[1]))
				LEX_INVAL_TOK_ERR();
			unsigned char val = (unsigned char)strtol(hex, NULL, 16);

			if (val > 0x7F)
				LEX_INVAL_TOK_ERR();

			APPEND_TO_BUFFER(val);
			state = STR_single;
			break;
		} // end STR_special_hex_s
		default:
			return 0;
		} // end switch
	} // end while
	return 0; // fallback
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
TokenPtr lexer(FILE *file)
{

	TokenPtr token = token_init();

	int state = START;
	int c;
	char buffer[MAX_BUFFER_LENGTH] = {0};
	size_t pos = 0; // buffer index
	// read first char
	c = fgetc(file);

	while (c != EOF)
	{
		switch (state)
		{
		case START:
		{
			if (c == '\n')
			{
				state = NEWLINE;
			}
			else if (isblank(c))
			{
				c = fgetc(file);
				continue;
			}
			else if (strchr("(){},.?:", c))
			{
				state = SPECIAL;
			}
			else if (strchr("!<=>", c))
			{ // lower,greater, (not) equals, leq, geq, assign
				state = CMP_OPERATOR;
			}
			else if (strchr("/+-*", c))
			{
				state = ARITHMETICAL;
			}
			else if (c == '\"')
			{
				state = STRING; // STRING/MULTILINE STRING
			}
			else if (isalpha(c) || c == '_')
			{
				state = IDENTIFIER;
				APPEND_TO_BUFFER(c); // start building identifier
			}
			else if (isdigit(c))
			{
				state = NUMERICAL;
			}
			else
			{
				APPEND_TO_BUFFER(c);
				token_update(token, buffer, NULL, -2);
				program_error(file, ERR_LEX, ERR_MSG_NOT_IMPLEMENTED, token);
			}
			token->type = state;
			break;
		} // end START

		case NEWLINE:
		{
			do
			{
				c = fgetc(file);
			} while (isspace(c)); // eat all whitespaces, newlines
			if (c == '/')
			{
				c = fgetc(file);
				if (c == '*')
				{
					state = BLOCK_COMMENT;
					break;
				}
				else if (c == '/')
				{
					state = COMMENT;
					break;
				}
			}
			ungetc(c, file); // push back non-newline char
			token_update(token, "\\n", NULL, NEWLINE);
			return token;
		} // end NEWLINE

		case IDENTIFIER:
		{
			c = fgetc(file);

			if (isalnum(c) || c == '_')
			{
				APPEND_TO_BUFFER(c); // build identifier
			}
			else
			{
				// char on the edge of the identif could be new valid tok e.g. a+b
				if (!isspace(c) && c != EOF && !strchr("(){},.<=>/+-*!/", c))
					LEX_INVAL_TOK_ERR();
				ungetc(c, file);
				// "_" invalid by itself
				if (pos == 1 && buffer[0] == '_')
				{
					LEX_INVAL_TOK_ERR();
				}
				// GLOBAL ID
				else if (pos > 1 && buffer[0] == '_' && buffer[1] == '_')
				{
					// must not start with digit
					if (!isalpha(buffer[2]) && buffer[2] != '\0')
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

		case STRING:
		{ // one " read
			if (!get_string(token, c, file))
				LEX_INVAL_TOK_ERR();
			return token;
		} // end STRING_SPECIAL

		case ARITHMETICAL:
		{
			int temp = c;	 // operator candidate
			c = fgetc(file); // lookahead

			if (temp == '/')
			{
				if (c == '/')
				{
					state = COMMENT;
					break;
				}
				else if (c == '*')
				{
					state = BLOCK_COMMENT;
					break;
				}
			}

			// For +, -, *, /
			APPEND_TO_BUFFER(temp);
			FINALIZE_TOK_ID(ARITHMETICAL); // push back lookahead + update
			return token;
		} // end ARITHMETICAL

		case COMMENT:
		{
			c = fgetc(file);
			token_update(token, "\\n", NULL, NEWLINE); // line comment is considered a newline
			if (c == '\n')
			{
				state = NEWLINE; // single-line comment = newline token
			}
			break;
		} // end COMMENT

		case BLOCK_COMMENT:
		{ // BLOCK COMMENTS can be nested
			if (!no_comment(file))
			{
				program_error(file, ERR_LEX, ERR_MSG_UNENCLOSED_COMM, token);
			}
			c = fgetc(file);
			state = START; // continue reading
			break;
		}

		case SPECIAL:
		{
			APPEND_TO_BUFFER(c);
			token_update(token, buffer, NULL, SPECIAL);
			return token;
		} // end SPECIAL

		case CMP_OPERATOR:
		{
			if (!get_cmp_op(token, c, file))
				LEX_INVAL_TOK_ERR();
			return token;
		} // end CMP_OPERATOR

		case NUMERICAL:
		{
			if (!numerizer(token, c, file))
				LEX_INVAL_TOK_ERR();
			return token;
		}

		default:
			program_error(file, ERR_MSG_NOT_IMPLEMENTED, ERR_LEX, token); // unknown token
		}
	} // while(!EOF)

	return token;
}