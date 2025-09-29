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
#define START            0
#define IDENTIFIER       1
#define ID_GLOBAL_VAR    2
#define NUMERICAL        3
#define STRING           4
#define STRING_SPECIAL   5
#define MULTILINE_STRING_1 6
#define MULTILINE_STRING_2 7

// Operators
#define CMP_OPERATOR     8
#define NOT_EQUAL        9
#define ARITHMETICAL     10
#define UNARY_PLUS       11
#define UNARY_MINUS      12

// Specials & punctuation
#define SPECIAL          13
#define NEWLINE          14

// Comments
#define COMMENT          15
#define BLOCK_COMMENT    16

// Keywords
#define KW_CLASS         17
#define KW_ELSE          18
#define KW_FALSE         19
#define KW_FOR           20
#define KW_IF            21
#define KW_IFJ           22
#define KW_IMPORT        23
#define KW_IS            24
#define KW_NULL          25
#define KW_NULL_TYPE     26
#define KW_NUM           27
#define KW_RETURN        28
#define KW_STATIC        29
#define KW_STRING        30
#define KW_TRUE          31
#define KW_VAR           32
#define KW_WHILE         33


struct KeywordEntry
{
    const char *word;
    int type;
};

TokenPtr lexer(FILE *file);

#endif // LEX_H