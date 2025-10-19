//////////////////////////////////////////////
// filename: lex.h	                        //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

/**
 * @file lex.h
 * @brief Header file for the lexical analyzer (scanner).
 *
 * This header provides:
 * - State definitions for the finite state machine (FSM).
 * - Function declaration for the scanner.
 *
 * Constants:
 * - `MAX_BUFFER_LENGTH`: Maximum length for tokens data/id storage.
 * - State definitions for various token types (e.g., identifiers, strings, operators).
 * - lex.c and common.c depend on these definitions. 
 */

#ifndef LEX_H
#define LEX_H

#include "common.h"

#define MAX_BUFFER_LENGTH 1024

#define FILE_END        -1

// Core tokens
#define START             0 // Lex States Only
#define IDENTIFIER        1
#define ID_GLOBAL_VAR     2
#define NUMERICAL         3
#define STRING            4
#define STRING_SPECIAL    5 // Lex States Only
#define MULTILINE_STRING_1 6
#define MULTILINE_STRING_2 7

// Operators
#define CMP_OPERATOR      8
#define NOT_EQUAL         9
#define ARITHMETICAL     10

// Specials & punctuation
#define SPECIAL          11
#define NEWLINE          12

// Comments
#define COMMENT          13 // Lex States Only
#define BLOCK_COMMENT    14 // Lex States Only

// Keywords
#define KW_CLASS         15
#define KW_ELSE          16
#define KW_FOR           17
#define KW_IF            18
#define KW_IFJ           19
#define KW_IMPORT        20
#define KW_IS            21
#define KW_NULL          22   // "null" literal
#define KW_NULL_TYPE     23   // "Null" type
#define KW_NUM           24
#define KW_RETURN        25
#define KW_STATIC        26
#define KW_STRING        27
#define KW_VAR           28
#define KW_WHILE         29

struct KeywordEntry
{
    const char *word;
    int type;
};

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
	{"Num", KW_NUM},
	{"String", KW_STRING},
	{"Null", KW_NULL_TYPE}
};

TokenPtr lexer(FILE *file);

#endif // LEX_H