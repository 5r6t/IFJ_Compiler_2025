#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "lex.h"
#include "ast.h"
#include <stdio.h>

typedef struct
{
    LiteralType liType;
    union
    {
        double num;
        char *str;
    };
} literal;

typedef struct
{
    char **parNames;
    int arrCnt;
    int arrCap;
} parArr;

typedef struct
{
    literal *items;
    int arrCnt;
    int arrCap;
} ArgArr;

typedef struct
{
    ASTnode **stmt;
    int stmtCnt;
    int stmtCap;
} stmtArr;

extern TokenPtr lookahead;
extern bool pending;

ASTptr parser(FILE *file);

ASTptr PROGRAM(TokenPtr *nextToken, FILE *file);
void PROLOG(TokenPtr *nextToken, FILE *file);
ASTptr CLASS(TokenPtr *nextToken, FILE *file);
ASTptr PAR(TokenPtr *nextToken, FILE *file, parArr *pA);
ASTptr NEXT_PAR(TokenPtr *nextToken, FILE *file, parArr *pA);
ASTptr FUNCTIONS(TokenPtr *nextToken, FILE *file, ASTptr programNode);
int FUNC_NAME(TokenPtr *nextToken, FILE *file);
ASTptr FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file, ASTptr functioNode);
int ARG_NAME(TokenPtr *nextToken, FILE *file);
int VAR_NAME(TokenPtr *nextToken, FILE *file); // , FILE *file
void ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr);
void NEXT_ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr);
ASTptr FUNC_BODY(TokenPtr *nextToken, FILE *file, ASTptr blockNode);
ASTptr RSA(TokenPtr *nextToken, FILE *file);
void FUNC_TYPE(TokenPtr *nextToken, FILE *file, ArgArr *argArr);

int nameHelperFunc(TokenPtr *nextToken, const target *target, size_t target_len, FILE *file);
int peek(const target *target, TokenPtr token, FILE *file);
void for_function(const target TARGE_SEQ[], FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN);
void parser_error(target target, TokenPtr token);
void advance(const target *target, TokenPtr *token, FILE *file);
TokenPtr getToken(FILE *file);
TokenPtr peekToken(FILE *file);

static inline void argArrInit(ArgArr *argArr)
{
    argArr->items = NULL;
    argArr->arrCap = 0;
    argArr->arrCnt = 0;
}

static inline void parArrInit(parArr *pA)
{
    pA->parNames = NULL;
    pA->arrCnt = 0;
    pA->arrCap = 0;
}

static inline void blockNodeInit(ASTptr blockNode)
{
    blockNode->type = AST_BLOCK;
    blockNode->block.stmt = NULL;
    blockNode->block.stmtCount = 0;
    blockNode->block.stmtCap = 0;
}

static inline void parArrAdd(parArr *pA, const char *name, FILE *file, TokenPtr token)
{
    if (pA->arrCnt == pA->arrCap)
    {
        int newCap;
        if (pA->arrCap == 0)
        {
            newCap = 4;
        }
        else
        {
            newCap = pA->arrCap * 2;
        }

        char **newArr = realloc(pA->parNames, newCap * sizeof(char *));
        if (!newArr)
        {
            program_error(file, 0, 0, token);
        }
        pA->parNames = newArr;
        pA->arrCap = newCap;
    }

    pA->parNames[pA->arrCnt] = my_strdup(name);
    pA->arrCnt++;
}

static inline void varNameAdd(ASTptr blockNode, ASTptr varNode, FILE *file, TokenPtr token)
{
    if (blockNode->block.stmtCount == blockNode->block.stmtCap)
    {
        int newCap;
        if (blockNode->block.stmtCap == 0)
        {
            newCap = 4;
        }
        else
        {
            newCap = blockNode->block.stmtCap * 2;
        }

        ASTptr *newArr = realloc(blockNode->block.stmt, newCap * sizeof(ASTptr));
        if (!newArr)
        {
            program_error(file, 0, 0, token);
        }
        blockNode->block.stmt = newArr;
        blockNode->block.stmtCap = newCap;
    }

    blockNode->block.stmt[blockNode->block.stmtCount] = varNode;
    blockNode->block.stmtCount++;
}

#endif