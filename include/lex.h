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

#define START 0
#define CMP_OPERATOR 1          // id=*, data=NULL, type=1
#define NOT_EQUAL 2             // id=*, data=NULL, type=2
#define SPECIAL 3               // id=*, data=NULL, type=3
#define ARITHMETICAL 4          // id=*, data=NULL, type=4
#define COMMENT 5               
#define STRING 6                // id=NULL, data=*, type=6
#define STRING_SPECIAL 7        
#define MULTILINE_STRING_1 8    
#define MULTILINE_STRING_2 9    // id=NULL, data=*, type=9
#define IDENTIFIER 10           // id=*, data=NULL, type=10
#define OUR_INT 11              // id=NULL, data=*, type=11
#define OUR_DOUBLE 12           // id=NULL, data=*, type=12
#define UNARY_PLUS 13
#define UNARY_MINUS 14
#define IN_BUILT_FUNC 15
#define KEYWORD 16

# include <common.h>

TokenPtr lexer(FILE *file);

#endif // LEX_H