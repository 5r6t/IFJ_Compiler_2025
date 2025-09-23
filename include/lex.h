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
 */

#ifndef LEX_H
#define LEX_H

#define MAX_BUFFER_LENGTH 1024

#define FILE_END -1
#define START 0
#define CMP_OPERATOR 1
#define NOT_EQUAL 2
#define SPECIAL 3       
#define ARITHMETICAL 4  
#define COMMENT 5               
#define STRING 6             
#define STRING_SPECIAL 7        
#define MULTILINE_STRING_1 8    
#define MULTILINE_STRING_2 9  
#define IDENTIFIER 10           
#define OUR_INT 11   
#define OUR_DOUBLE 12          
#define UNARY_PLUS 13
#define UNARY_MINUS 14
#define IN_BUILT_FUNC 15

#define KW_CLASS 16
#define KW_IF 17
#define KW_ELSE 18
#define KW_IS 19
#define KW_NULL 20
#define KW_RETURN 21
#define KW_VAR 22
#define KW_WHILE 23
#define KW_IFJ 24
#define KW_STATIC 25
#define KW_TRUE 26
#define KW_FALSE 27
#define KW_NUM 28
#define KW_STRING 29
#define KW_NULL_TYPE 30

#define NEWLINE 31
#define ID_GLOBAL_VAR 32 // identifier for global variable

# include <common.h>

struct KeywordEntry {
    const char *word;
    int type;
};

TokenPtr lexer(FILE *file);

#endif // LEX_H