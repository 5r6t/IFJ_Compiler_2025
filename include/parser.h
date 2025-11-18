#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "lex.h"
#include "ast.h"
#include <stdio.h>

ASTptr parser(FILE *file);

ASTptr PROGRAM(TokenPtr *nextToken, FILE *file);
int PROLOG(TokenPtr *nextToken, FILE *file);
ASTptr CLASS(TokenPtr *nextToken, FILE *file);
ASTptr FUNCTIONS(TokenPtr *nextToken, FILE *file, ASTptr programNode);
int FUNC_NAME(TokenPtr *nextToken, FILE *file);
ASTptr FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file, ASTptr functioNode);
ASTptr PAR(TokenPtr *nextToken, FILE *file);
ASTptr NEXT_PAR(TokenPtr *nextToken, FILE *file);
ASTptr FUNC_BODY(TokenPtr *nextToken, FILE *file);
ASTptr RSA(TokenPtr *nextToken, FILE *file);
ASTptr FUNC_TYPE(TokenPtr *nextToken, FILE *file);
ASTptr ARG(TokenPtr *nextToken, FILE *file);
ASTptr NEXT_ARG(TokenPtr *nextToken, FILE *file);
ASTptr ARG_NAME(TokenPtr *nextToken, FILE *file);
ASTptr VAR_NAME(TokenPtr *nextToken, FILE *file);

int nameHelperFunc(TokenPtr *nextToken, const target *target, size_t target_len);
int peek(const target *target, TokenPtr token);
void for_function(const target TARGE_SEQ[], FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN);
void parser_error(target target, TokenPtr token);
void advance(const target *target, TokenPtr *token, FILE *file);

#endif