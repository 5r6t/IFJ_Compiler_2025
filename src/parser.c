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

/*NOTES:
   - once in time should call semantic analyze to make AST from simulated derivation tree -> quit confused how to do that
*/

void parser(FILE *file)
{
    TokenPtr nextToken = lexer(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    PROGRAM(nextToken, file);
    /*if (PROGRAM(nextToken, file))
    {
        return 0;
    }
    else
    {
        return 1; // change to error code that is most fitting
    }*/
}

int PROGRAM(TokenPtr *nextToken, FILE *file) // maybe I should return ast nodes (required to have tree -> define in semantic analyze)
{
    PROLOG(nextToken, file);

    CLASS(nextToken, file);

    return 0;
}

int PROLOG(TokenPtr *nextToken, FILE *file)
{
    // TODO change this to be more effective using function while_function
    static const target PROLOG_TARGET[] = {
        {IDENTIFIER, "import"}, // data are used for expression not identificator
        {STRING, "ifj25"},
        {IDENTIFIER, "for"},
        {KW_IFJ, "Ifj"},
        {NEWLINE, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(PROLOG_TARGET) / sizeof(PROLOG_TARGET[0]);

    for_function(PROLOG_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    return 0;
}

int CLASS(TokenPtr *nextToken, FILE *file)
{
    // TODO make while and array filled with "class Program { EOL". While would iterated until array is passted then function FUNCTIONS is called and after that check for }

    static const target CLASS_TARGET[] =
        {
            {KW_CLASS, "class"},
            {IDENTIFIER, "Program"},
            {SPECIAL, "{"},
            {NEWLINE, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(CLASS_TARGET) / sizeof(CLASS_TARGET[0]);
    for_function(CLASS_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    FUNCTIONS(nextToken, file);
    // nextToken = lexer(file);

    static const target CLASS_TARGET_END = {SPECIAL, "}"};
    match(&CLASS_TARGET_END, nextToken);

    return 0;
}

int FUNCTIONS(TokenPtr *nextToken, FILE *file)
{

    if ((*nextToken)->type == KW_STATIC)
    {
        static const target FUNCTIONS_FIRST = {KW_STATIC, "static"};
        advance(&FUNCTIONS_FIRST, nextToken, file);
        FUNC_NAME(nextToken, file);        // dont forget to iterate nextToken inside this function!!!
        FUNC_GET_SET_DEF(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        FUNCTIONS(nextToken, file);
        nextToken = lexer(file);
        return 0;
    }
    else if ((*nextToken)->type == SPECIAL)
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

int FUNC_NAME(TokenPtr *nextToken, FILE *file)
{
    static const target FUNC_NAME_TARGET = {IDENTIFIER, NULL};
    advance(&FUNC_NAME_TARGET, nextToken, file); // is nextToken pointer?
    return 0;
}

int FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file)
{
    target FUNC_DEF = {SPECIAL, "("};

    target FUNC_GET = {SPECIAL, "{"};

    target FUNC_SET_SEQ[] =
        {
            {CMP_OPERATOR, "="},
            {SPECIAL, "("},
            {IDENTIFIER, NULL},
            (SPECIAL, ")"),
            {SPECIAL, "{"},
            {NEWLINE, NULL}};

    target FUNC_DEF_SEQ[] =
        {
            {SPECIAL, ")"},
            {SPECIAL, "{"},
            {NEWLINE, NULL}};

    target FUNC_GET_SET_DEF_END[] = // last two lexem in rules for FUNC_GET_SET_DEF are same -> } EOL
        {
            {SPECIAL, "}"},
            {NEWLINE, NULL}};

    size_t FUNC_SET_SEQ_LEN = sizeof(FUNC_SET_SEQ) / sizeof(FUNC_SET_SEQ[0]);
    size_t FUNC_DEF_SEQ_LEN = sizeof(FUNC_DEF_SEQ) / sizeof(FUNC_DEF_SEQ[0]);
    size_t FUNC_GET_SET_DEF_END_LEN = sizeof(FUNC_GET_SET_DEF_END) / sizeof(FUNC_GET_SET_DEF_END[0]);

    if (match(&FUNC_DEF, nextToken) == 0) // definition of function -> should call semantic analyzer to check if function id is already used -> beware of shadowing!!!
    {
        nextToken = lexer(file);
        PAR(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_DEF_SEQ, file, nextToken, FUNC_DEF_SEQ_LEN);
        FUNC_BODY(); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else if (match(&FUNC_GET, nextToken) == 0) // getter -> should I call semantic analyzer here too?
    {
        nextToken = lexer(file);
        FUNC_GET.data = NULL;
        // FUNC_DEF.type = EOL;
        advance(&FUNC_GET, nextToken, file);
        FUNC_BODY(); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else if ((*nextToken)->type == CMP_OPERATOR) // setter
    {
        for_function(FUNC_SET_SEQ, file, nextToken, FUNC_SET_SEQ_LEN);
        FUNC_BODY(); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else // should call program error
    {
        fprintf(stderr, "error 2");
        exit(2);
    }
}

int PAR(TokenPtr *nextToken, FILE *file)
{
    static const target PAR_FIRST = {IDENTIFIER, NULL};
    static const target PAR_FOLLOW = {SPECIAL, ")"};
    if ((*nextToken)->type == IDENTIFIER)
    {
        advance(&PAR_FIRST, nextToken, file);
        NEXT_PAR(); // dont forget to iterate nextToken inside this function!!!
        return 0;
    }
    else if ((*nextToken)->type == SPECIAL)
    {
        match(&PAR_FOLLOW, nextToken);
        return 0;
    }
    else
    {
        parser_error(PAR_FIRST, nextToken);
        exit(2);
    }
}

int FUNC_BODY()
{
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
        if (target->data != NULL)
        {
            if (token->data == NULL)
            {
                fprintf(stderr, "SEGFAULT, token pointer is empty and you are trying reach something you can`t idiot\n");
                exit;
            }

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
void for_function(target TARGE_SEQ[], FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN)
{
    for (int i = 0; i < TARGE_SEQ_LEN; i++)
    {
        // match(&TARGE_SEQ[i], nextToken);
        // nextToken = lexer(file);
        advance(&TARGE_SEQ[i], nextToken, file);
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
void advance(const target *target, TokenPtr *token, FILE *file)
{
    match(target, token);
    *token = lexer(file);
}
