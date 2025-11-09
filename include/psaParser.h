#ifndef PSAPARSER_H
#define PSAPARSER_H

#include "common.h"
#include "lex.h"

typedef enum
{
    PRE_TAB_NULL = 0,
    PRE_TAB_LESS = 1,
    PRE_TAB_GREATER = 2,
    PRE_TAB_EQUAL = 3
} PrecedentTableRel;

// Postion of operators in precedence table
#define MULL 0
#define DIV 1
#define ADD 2
#define SUB 3
#define EQUAL 4
#define NEQUAL 5
#define LESS 6
#define GREATER 7
#define LSEQ 8
#define GREQ 9
#define LPAR 10
#define RPAR 11
#define ID 12
#define IS 13
#define END 14

int parse_expression(TokenPtr *nextToken, FILE *file, target *endIfExp);

#endif