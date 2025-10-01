#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "lex.h"
#include <stdio.h>

int PROGRAM(TokenPtr *nextToken, FILE *file);
int PROLOG(TokenPtr *nextToken, FILE *file);
int CLASS(TokenPtr *nextToken, FILE *file);
int FUNCTIONS(TokenPtr *nextToken, FILE *file);
int FUNC_NAME(TokenPtr *nextToken, FILE *file);
int FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file);
int PAR(TokenPtr *nextToken, FILE *file);
int NEXT_PAR(TokenPtr *nextToken, FILE *file);
int FUNC_BODY(TokenPtr *nextToken, FILE *file);
int RSA(TokenPtr *nextToken, FILE *file);
int FUNC_TYPE(TokenPtr *nextToken, FILE *file);
int ARG(TokenPtr *nextToken, FILE *file);
int NEXT_ARG(TokenPtr *nextToken, FILE *file);
int ARG_NAME(TokenPtr *nextToken, FILE *file);
int VAR_NAME(TokenPtr *nextToken, FILE *file);

int nameHelperFunc(TokenPtr *nextToken, const target *target, size_t target_len);
int peek(const target *target, TokenPtr token);
void for_function(const target TARGE_SEQ[], FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN);
void parser_error(target target, TokenPtr token);
void advance(const target *target, TokenPtr *token, FILE *file);

#endif