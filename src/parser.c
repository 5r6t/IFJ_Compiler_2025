//////////////////////////////////////////////
// filename: parser.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/common.h"
#include "../include/lex.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parser(FILE *file)
{
    TokenPtr nextToken = lexer(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    if (PROGRAM(nextToken, file))
    {
        return 0;
    }
    else
    {
        return 1; // change to error code that is most fitting
    }
}

int PROGRAM(TokenPtr nextToken, FILE *file)
{
    PROLOG(nextToken, file);

    CLASS(nextToken, file);

    return 0;
    /*if (PROLOG(nextToken, file))
    {
        if (CLASS(nextToken, file))
        {
            return 0;
        }
        else
        {
            return 1; // change to error code that is most fitting -> code will be 2. Should think about how to handle error better -> maybe use function here?( I wouldn`t need to have returned type of int)
        }
    }
    else
    {
        return 1; // change to error code that is most fitting
    }*/
}

int PROLOG(TokenPtr nextToken, FILE *file)
{
    // TODO change this to be more effective using function while_function
    static const target PROLOG_TARGET[] = {
        {IDENTIFIER, "import"},
        {STRING, "ifj25"},
        {IDENTIFIER, "for"},
        {KW_IFJ, "Ifj"},
        //{EOL, NULL}
    };

    static const size_t PROLOG_TARGET_LEN = sizeof(PROLOG_TARGET) / sizeof(PROLOG_TARGET[0]);

    for_function(PROLOG_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    return 0;
}

int CLASS(TokenPtr nextToken, FILE *file)
{
    // TODO make while and array filled with "class Program { EOL". While would iterated until array is passted then function FUNCTIONS is called and after that check for }

    static const target CLASS_TARGET[] =
        {
            {KW_CLASS, "class"},
            {IDENTIFIER, "Program"},
            {SPECIAL, "{"}
            //{EOL, NULL},
        };

    static const size_t PROLOG_TARGET_LEN = sizeof(CLASS_TARGET) / sizeof(CLASS_TARGET[0]);
    for_function(CLASS_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    FUNCTIONS(nextToken, file);
    // nextToken = lexer(file);

    static const target CLASS_TARGET_END = {SPECIAL, "}"};
    match(&CLASS_TARGET_END, nextToken);

    return 0;
}

int FUNCTIONS(TokenPtr nextToken, FILE *file)
{

    if (nextToken->type == KW_STATIC)
    {
        static const target FUNCTIONS_FIRST = {KW_STATIC, "static"};
        nextToken = advance(&FUNCTIONS, nextToken, file);
        FUNC_NAME(nextToken, file);        // dont forget to iterate nextToken inside this function!!!
        FUNC_GET_SET_DEF(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        FUNCTIONS(nextToken, file);
        nextToken = lexer(file);
        return 0;
    }
    else if (nextToken->type == SPECIAL)
    {
        static const target FUNCTIONS_FOLLOW = {SPECIAL, "}"};
        match(&FUNCTIONS_FOLLOW, nextToken);
        return 0;
    }
    else // should call program error
    {
        fprintf(stderr, "error 2");
        exit(2);
    }
}

int FUNC_NAME(TokenPtr nextToken, FILE *file)
{
    static const target FUNC_NAME_TARGET = {IDENTIFIER, NULL};
    nextToken = advance(&FUNC_NAME_TARGET, nextToken, file); // is nextToken pointer?
    return 0;
}

int FUNC_GET_SET_DEF(TokenPtr nextToken, FILE *file)
{
    target FUNC_DEF = {SPECIAL, "("};

    target FUNC_GET = {SPECIAL, "{"};

    target FUNC_DEF_SEQ[] =
        {
            {SPECIAL, ")"},
            {SPECIAL, "{"},
            //{EOL, NULL}
        };

    target FUNC_GET_SET_DEF_END[] = // last two lexem in rules for FUNC_GET_SET_DEF are same -> } EOL
        {
            {SPECIAL, "}"}
            //{EOL, NULL}
        };

    size_t FUNC_DEF_SEQ_LEN = sizeof(FUNC_DEF_SEQ) / sizeof(FUNC_DEF_SEQ[0]);
    size_t FUNC_GET_SET_DEF_END_LEN = sizeof(FUNC_GET_SET_DEF_END) / sizeof(FUNC_GET_SET_DEF_END[0]);

    if (match(&FUNC_DEF, nextToken) == 0)
    {
        nextToken = lexer(file);
        PAR(); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_DEF_SEQ, file, nextToken, FUNC_DEF_SEQ_LEN);
        FUNC_BODY(); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else if (match(&FUNC_GET, nextToken) == 0)
    {
        nextToken = lexer(file);
        FUNC_GET.data = NULL;
        // FUNC_DEF.type = EOL;
        nextToken = advance(&FUNC_GET, nextToken, file);
        FUNC_BODY(); // dont forget to iterate nextToken inside this function!!!
    }
}

// using strcmp compares target string with token. If token isn`t same as target string error is send to stderr output
/**
 * @brief checks if expected terminal and actual terminal are same
 * @param target expected terminal
 * @param token actual terminal
 *
 * @return wrong terminal will trigger stderr and program will end, correct terminal will return 0
 */
int match(const target *target, TokenPtr token)
{
    if (target->type != token->type)
    {
        parser_error(*target, token);
    }
    else
    {
        if (target->type == (IDENTIFIER || SPECIAL) && target->data != NULL)
        {
            if (strcmp(target->data, token->data) != 0)
            {
                parser_error(*target, token);
            }
        }
        return 0;
    }
    return 0;
}

// helping function for more pleasing way to check matches and for updating nextToken
void for_function(target TARGE_SEQ[], FILE *file, TokenPtr nextToken, size_t TARGE_SEQ_LEN)
{
    for (int i = 0; i < TARGE_SEQ_LEN; i++)
    {
        // match(&TARGE_SEQ[i], nextToken);
        // nextToken = lexer(file);
        nextToken = advance(&TARGE_SEQ[i], nextToken, file);
    }
}

void parser_error(target target, TokenPtr token)
{
    fprintf(stderr, "error code 2: syntax analyze error, expected: '%s', got '%s' \n", target.data, token->data);
    exit(2);
}

/**
 * @brief check lookahead with function match and then iterate it
 * @return return next lookahead token
 */
TokenPtr advance(const target *target, TokenPtr token, FILE *file)
{
    match(&target, token);
    token = lexer(file);
    return token;
}
