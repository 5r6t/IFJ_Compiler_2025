//////////////////////////////////////////////
// filename: lex.c	                   	//
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

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