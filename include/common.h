//////////////////////////////////////////////
// filename: utils.h	                    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#ifndef COMMON_H
#define COMMON_H

#ifdef  DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token {
    char *id;
    int type;
    char *data;
} *TokenPtr;

#endif