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
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/psaParser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*NOTES:
   - once in time should call semantic analyze to make AST from simulated derivation tree -> quit confused how to do that
   ├─ shouldn't it create an AST during analysis, i.e., when it recognizes the structure, it creates a suitable AST node? -> A: yes, it will be call at the end of function
   └─ maybe change funciton types from int to ASTptr so we can assemble the tree -> A: yes but first i want to finish LL rules
   - problem with arg_name using peek function check if there is valid token type if yes token will be processed (token will be used to created AST node ), must thing of way how to processed all arg at once or not
*/

/* LL1:

PROGRAM ::= PROLOG CLASS
PROLOG ::= import "ifj25" for Ifj /
CLASS ::= class Program { / FUNCTIONS }
FUNCTIONS ::= static FUNC_GET_SET_DEF FUNCTIONS
FUNCTIONS ::= ''
FUNC_GET_SET_DEF ::= ( PAR ) { / FUNC_BODY } /
FUNC_GET_SET_DEF ::= { / FUNC_BODY } /
FUNC_GET_SET_DEF ::= = ( id ) { / FUNC_BODY } /

PAR ::= ''
PAR ::= id NEXT_PAR
NEXT_PAR ::= , id NEXT_PAR
NEXT_PAR ::= ''

ARG ::= ''
ARG ::= ARG_NAME NEXT_ARG
NEXT_ARG ::= ''
NEXT_ARG ::= , ARG_NAME NEXT_ARG

ARG_NAME ::= int
ARG_NAME ::= string
ARG_NAME ::= float
ARG_NAME ::= id
ARG_NAME ::= global_id

EXPRESSION ::= int
EXPRESSION ::= string
EXPRESSION ::= float


FUNC_NAME ::= id
FUNC_NAME ::= Ifj . id

VAR_NAME ::= id
VAR_NAME ::= global_id

FUNC_BODY ::= ''
FUNC_BODY ::= VAR_DECL FUNC_BODY
FUNC_BODY ::= VAR_ASS_CALL_GET FUNC_BODY
FUNC_BODY ::= IF_STAT FUNC_BODY
FUNC_BODY ::= WHILE FUNC_BODY
FUNC_BODY ::= RETURN FUNC_BODY
FUNC_BODY ::= { / FUNC_BODY } / FUNC_BODY

VAR_DECL ::= var VAR_NAME /
VAR_ASS_CALL_GET ::= VAR_NAME = RSA
RSA ::= EXPRESSION /
RSA ::= id FUNC_TYPE /
RSA ::= Ifj . id ( ARG ) /
FUNC_TYPE ::= ''
FUNC_TYPE ::= ( ARG )
IF_STAT ::= if ( EXPRESSION ) { / FUNC_BODY } else { / FUNC_BODY } /
WHILE ::= while ( EXPRESSION ) { / FUNC_BODY } /
RETURN ::= return EXPRESSION /

 */

void parser(FILE *file) // change return type to ASTnode
{
    TokenPtr nextToken = lexer(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    PROGRAM(&nextToken, file);
}

int PROGRAM(TokenPtr *nextToken, FILE *file) // maybe I should return ast nodes (required to have tree -> define in semantic analyze)
{
    PROLOG(nextToken, file);

    CLASS(nextToken, file);

    return 0;
}

int PROLOG(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    // TODO change this to be more effective using function while_function
    static const target PROLOG_TARGET[] = {
        {KW_IMPORT, "import", NULL}, // data are used for expression not identificator
        {STRING, "ifj25", NULL},
        {KW_FOR, "for", NULL},
        {KW_IFJ, "Ifj", NULL},
        {NEWLINE, NULL, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(PROLOG_TARGET) / sizeof(PROLOG_TARGET[0]);

    for_function(PROLOG_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    return 0;
}

int CLASS(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    // TODO make while and array filled with "class Program { EOL". While would iterated until array is passted then function FUNCTIONS is called and after that check for }

    static const target CLASS_TARGET[] =
        {
            {KW_CLASS, "class", NULL},
            {IDENTIFIER, "Program", NULL},
            {SPECIAL, "{", NULL},
            {NEWLINE, NULL, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(CLASS_TARGET) / sizeof(CLASS_TARGET[0]);
    for_function(CLASS_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    // create ASTnode for class Program - TODO
    /*
    ASTptr class_node = (ASTptr)malloc(sizeof(struct ASTnode));
    ASTptr->type = CLASS_NODE;
    ASTptr->program.child = FUNCTIONS(nextToken, file);
    */

    FUNCTIONS(nextToken, file);
    // nextToken = lexer(file);

    static const target CLASS_TARGET_END = {SPECIAL, "}", NULL};
    advance(&CLASS_TARGET_END, nextToken, file);

    return 0;
}

int FUNCTIONS(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target FUNCTIONS_FOLLOW = {SPECIAL, "}", NULL};

    if ((*nextToken)->type == KW_STATIC)
    {
        /* // TODO -> need to talk with ya about this
        ASTptr func_node = (ASTptr)malloc(sizeof(struct ASTnode));
        func_node->type = FUNC_NODE;
        func_node->func.name = FUNC_NAME(nextToken, file);
        func_node->func.params = FUNC_GET_SET_DEF(nextToken, file);
        func_node->func.body = FUNCTIONS(nextToken, file);
        */
        static const target FUNCTIONS_FIRST = {KW_STATIC, "static", NULL};
        advance(&FUNCTIONS_FIRST, nextToken, file);
        FUNC_NAME(nextToken, file);        // dont forget to iterate nextToken inside this function!!!
        FUNC_GET_SET_DEF(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        FUNCTIONS(nextToken, file);
        return 0;
        // return func_node;
    }
    else if (peek(&FUNCTIONS_FOLLOW, *nextToken)) // epsilon
    {
        return 0;
    }
    else // should call program error
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int FUNC_NAME(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    /*static const target FUNC_NAME_SEQ[] =
        {
            {KW_IFJ, NULL, NULL},
            {SPECIAL, ".", NULL},
            {IDENTIFIER, NULL, NULL}};

    size_t FUNC_NAME_SEQ_LEN = sizeof(FUNC_NAME_SEQ) / sizeof(FUNC_NAME_SEQ[0]);*/
    // static const target FUNC_NAME_TARGET = {IDENTIFIER, NULL};
    if ((*nextToken)->type == IDENTIFIER)
    {
        // advance(&FUNC_NAME_TARGET, nextToken, file); // is nextToken pointer?
        *nextToken = lexer(file);
        return 0;
    }
    /*else if ((*nextToken)->type == KW_IFJ)
    {
        for_function(FUNC_NAME_SEQ, file, nextToken, FUNC_NAME_SEQ_LEN);
        return 0;
    }*/

    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    target FUNC_DEF = {SPECIAL, "(", NULL};

    target FUNC_GET = {SPECIAL, "{", NULL};

    static const target FUNC_SET_SEQ[] =
        {
            {CMP_OPERATOR, "=", NULL},
            {SPECIAL, "(", NULL},
            {IDENTIFIER, NULL, NULL},
            {SPECIAL, ")", NULL},
            {SPECIAL, "{", NULL},
            {NEWLINE, NULL, NULL}};

    target FUNC_DEF_SEQ[] =
        {
            {SPECIAL, ")", NULL},
            {SPECIAL, "{", NULL},
            {NEWLINE, NULL, NULL}};

    target FUNC_GET_SET_DEF_END[] = // last two lexem in rules for FUNC_GET_SET_DEF are same -> } EOL
        {
            {SPECIAL, "}", NULL},
            {NEWLINE, NULL, NULL}};

    size_t FUNC_SET_SEQ_LEN = sizeof(FUNC_SET_SEQ) / sizeof(FUNC_SET_SEQ[0]);
    size_t FUNC_DEF_SEQ_LEN = sizeof(FUNC_DEF_SEQ) / sizeof(FUNC_DEF_SEQ[0]);
    size_t FUNC_GET_SET_DEF_END_LEN = sizeof(FUNC_GET_SET_DEF_END) / sizeof(FUNC_GET_SET_DEF_END[0]);

    if (peek(&FUNC_DEF, *nextToken) == 0) // definition of function -> should call semantic analyzer to check if function id is already used -> beware of shadowing!!!
    {
        *nextToken = lexer(file);
        PAR(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_DEF_SEQ, file, nextToken, FUNC_DEF_SEQ_LEN);
        FUNC_BODY(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else if (peek(&FUNC_GET, *nextToken) == 0) // getter -> should I call semantic analyzer here too? FATAL OVERSIGTH: THIS WILL KILL THE PROGRAM WITHOUT CHCEKING ALL RULES FIRST
    {
        *nextToken = lexer(file);
        FUNC_GET.data = NULL;
        FUNC_DEF.type = NEWLINE;
        advance(&FUNC_GET, nextToken, file);
        FUNC_BODY(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else if ((*nextToken)->type == CMP_OPERATOR) // setter
    {
        for_function(FUNC_SET_SEQ, file, nextToken, FUNC_SET_SEQ_LEN);
        FUNC_BODY(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int PAR(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target PAR_FIRST = {IDENTIFIER, NULL, NULL};
    static const target PAR_FOLLOW = {SPECIAL, ")", NULL};
    if ((*nextToken)->type == IDENTIFIER)
    {
        advance(&PAR_FIRST, nextToken, file);
        NEXT_PAR(nextToken, file); // dont forget to iterate nextToken inside this function!!!
        return 0;
    }
    else if ((*nextToken)->type == SPECIAL)
    {
        peek(&PAR_FOLLOW, *nextToken);
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int NEXT_PAR(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target NEXT_PAR_FIRST = {SPECIAL, ",", NULL};
    static const target NEXT_PAR_FOLLOW = {SPECIAL, ")", NULL};
    if ((*nextToken)->type == SPECIAL)
    {
        advance(&NEXT_PAR_FIRST, nextToken, file);
        NEXT_PAR(nextToken, file);
        return 0;
    }
    else if (peek(&NEXT_PAR_FOLLOW, *nextToken)) // epsilon
    {
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int FUNC_BODY(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target DUMMY_EXPRESSION = {IDENTIFIER, NULL, NULL};
    static const target END_IF_EXP = {SPECIAL, ")", NULL};
    static const target RETURN_FIRST = {KW_RETURN, NULL, NULL};
    static const target FUNC_BODY_END = {NEWLINE, NULL, NULL};
    static const target FUNC_BODY_FOLLOW = {SPECIAL, "}", NULL};
    static const target VAR_ASS_CALL_GET = {CMP_OPERATOR, "=", NULL};

    static const target FUNC_INTRO_SEQ[] =
        {
            {SPECIAL, "(", NULL},
            {NEWLINE, NULL, NULL}};

    /*static const target FUNC_BODY_DECL_SEQ[] =
        {
            {KW_VAR, "var", NULL},
            {IDENTIFIER, NULL, NULL},
            {NEWLINE, NULL, NULL}};*/

    /*static const target VAR_ASS_CALL_GET_SEQ[] =
        {
            {IDENTIFIER, NULL, NULL},
            {CMP_OPERATOR, "=", NULL}};*/

    static const target IF_STATMENT_START_SEQ[] =
        {
            {KW_IF, "if", NULL},
            {SPECIAL, "(", NULL}};

    static const target IF_STATMENT_MIDDLE_SEQ[] =
        {
            {SPECIAL, ")", NULL},
            {SPECIAL, "{", NULL},
            {NEWLINE, NULL, NULL}};

    static const target IF_STATMENT_ELSE_BRANCH_SEQ[] =
        {
            {SPECIAL, "}", NULL},
            {KW_ELSE, NULL, NULL},
            {SPECIAL, "{", NULL},
            {NEWLINE, NULL, NULL}};

    static const target END_SEQ[] =
        {
            {SPECIAL, "}", NULL},
            {NEWLINE, NULL, NULL}};

    static const target WHILE_START_SEQ[] =
        {
            {KW_WHILE, NULL, NULL},
            {SPECIAL, "(", NULL}};

    size_t IF_STATMENT_ELSE_BRANCH_SEQ_LEN = sizeof(IF_STATMENT_ELSE_BRANCH_SEQ) / sizeof(IF_STATMENT_ELSE_BRANCH_SEQ[0]);
    size_t IF_STATMENT_MIDDLE_SEQ_LEN = sizeof(IF_STATMENT_MIDDLE_SEQ) / sizeof(IF_STATMENT_MIDDLE_SEQ[0]);
    size_t IF_STATMENT_START_SEQ_LEN = sizeof(IF_STATMENT_START_SEQ) / sizeof(IF_STATMENT_START_SEQ[0]);
    // size_t VAR_ASS_CALL_GET_SEQ_LEN = sizeof(VAR_ASS_CALL_GET_SEQ) / sizeof(VAR_ASS_CALL_GET_SEQ[0]);
    // size_t FUNC_BODY_DECL_SEQ_LEN = sizeof(FUNC_BODY_DECL_SEQ) / sizeof(FUNC_BODY_DECL_SEQ[0]);
    size_t WHILE_START_SEQ_LEN = sizeof(WHILE_START_SEQ) / sizeof(WHILE_START_SEQ[0]);
    size_t FUNC_INTRO_SEQ_LEN = sizeof(FUNC_INTRO_SEQ) / sizeof(FUNC_INTRO_SEQ[0]);
    size_t END_SEQ_LEN = sizeof(END_SEQ) / sizeof(END_SEQ[0]);

    if ((*nextToken)->type == KW_VAR) // rework -> Add var name as function -> did i already done that?
    {
        // for_function(FUNC_BODY_DECL_SEQ, file, nextToken, FUNC_BODY_DECL_SEQ_LEN);
        *nextToken = lexer(file);
        VAR_NAME(nextToken, file);
        advance(&FUNC_BODY_END, nextToken, file);
        FUNC_BODY(nextToken, file);
        return 0;
    }
    else if (VAR_NAME(nextToken, file)) // beware GLOBAL ID can be here used as name so this must function should work different
    {
        // for_function(VAR_ASS_CALL_GET_SEQ, file, nextToken, VAR_ASS_CALL_GET_SEQ_LEN);
        //  here should be creation of node for ast or something, should discus with Honza
        advance(&VAR_ASS_CALL_GET, nextToken, file);
        RSA(nextToken, file);
        return 0;
    }
    else if ((*nextToken)->type == KW_IF) // if statment
    {
        for_function(IF_STATMENT_START_SEQ, file, nextToken, IF_STATMENT_START_SEQ_LEN);
        // Here I will give control to PSA, for now I will use dummy expresion
        // advance(&DUMMY_EXPRESSION, nextToken, file);
        parse_expression(nextToken, file, &END_IF_EXP);
        for_function(IF_STATMENT_MIDDLE_SEQ, file, nextToken, IF_STATMENT_MIDDLE_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        for_function(IF_STATMENT_ELSE_BRANCH_SEQ, file, nextToken, IF_STATMENT_ELSE_BRANCH_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        return 0;
    }
    else if ((*nextToken)->type == KW_WHILE)
    {
        for_function(WHILE_START_SEQ, file, nextToken, WHILE_START_SEQ_LEN);
        // Here I will give control to PSA, for now I will use dummy expresion
        advance(&DUMMY_EXPRESSION, nextToken, file);
        for_function(IF_STATMENT_MIDDLE_SEQ, file, nextToken, IF_STATMENT_MIDDLE_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        return 0;
    }
    else if ((*nextToken)->type == KW_RETURN) // mozne miesto na prerobenie
    {
        advance(&RETURN_FIRST, nextToken, file);
        // Here I will give control to PSA, for now I will use dummy expresion
        advance(&DUMMY_EXPRESSION, nextToken, file);
        advance(&FUNC_BODY_END, nextToken, file);
        FUNC_BODY(nextToken, file);
        return 0;
    }
    else if ((*nextToken)->type == SPECIAL)
    {
        for_function(FUNC_INTRO_SEQ, file, nextToken, FUNC_INTRO_SEQ_LEN);
        FUNC_BODY(nextToken, file);
        advance(&FUNC_BODY_END, nextToken, file);
        FUNC_BODY(nextToken, file);
        return 0;
    }
    else if (peek(&FUNC_BODY_FOLLOW, *nextToken)) // epsilon
    {
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int RSA(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target IN_BUILT_FUNC_SEQ[] =
        {
            {SPECIAL, ".", NULL},
            {IDENTIFIER, NULL, NULL},
            {SPECIAL, "(", NULL}};

    static const target END_SEQ[] =
        {
            {SPECIAL, ")", NULL},
            {NEWLINE, NULL, NULL}};

    size_t IN_BUILT_FUNC_SEQ_LEN = sizeof(IN_BUILT_FUNC_SEQ) / sizeof(IN_BUILT_FUNC_SEQ[0]);
    size_t END_SEQ_LEN = sizeof(END_SEQ) / sizeof(END_SEQ[0]);

    if ((*nextToken)->type == IDENTIFIER)
    {
        (*nextToken) = lexer(file);
        FUNC_TYPE(nextToken, file);
        if ((*nextToken)->type != NEWLINE)
        {
            // error capture
            exit(2);
        }
        return 0;
    } // there can be a expression -> give control to PSA
    else if ((*nextToken)->type == KW_IFJ)
    {
        for_function(IN_BUILT_FUNC_SEQ, file, nextToken, IN_BUILT_FUNC_SEQ_LEN);
        ARG(nextToken, file);
        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int FUNC_TYPE(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target FUNC_TYPE_FIRST = {SPECIAL, "(", NULL};
    static const target FUNC_TYPE_NEXT = {SPECIAL, ")", NULL};
    if ((*nextToken)->type == SPECIAL)
    {
        advance(&FUNC_TYPE_FIRST, nextToken, file);
        ARG(nextToken, file);
        advance(&FUNC_TYPE_NEXT, nextToken, file);
        return 0;
    }
    else if ((*nextToken)->type == NEWLINE)
    {
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int ARG(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target ARG_FOLLOW = {SPECIAL, ")", NULL};

    if (ARG_NAME(nextToken, file))
    {
        // create node
        *nextToken = lexer(file);
        NEXT_ARG(nextToken, file); // nextToken was itareted in this function
        return 0;
    }
    else if (peek(&ARG_FOLLOW, *nextToken)) // epsilon
    {
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

int NEXT_ARG(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target NEXT_ARG_FIRST = {SPECIAL, ",", NULL}; // look at it again please
    static const target NEXT_ARG_FOLLOW = {SPECIAL, ")", NULL};
    if (peek(&NEXT_ARG_FIRST, *nextToken))
    {
        *nextToken = lexer(file);
        ARG_NAME(nextToken, file);
        *nextToken = lexer(file);
        return 0;
    }
    else if (peek(&NEXT_ARG_FOLLOW, *nextToken)) // epsilon
    {
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1; // unnecessary return -> gcc will cry if omitted
}

int ARG_NAME(TokenPtr *nextToken, FILE *file) // THIS NEEDS TO BE INT -> NODE WILL BE CREATE IN FUNCTION ARG AND NEXT_ARG
{
    static const target ARG_NAME_FIRST[] = {
        {NUMERICAL, NULL, NULL},
        {IDENTIFIER, NULL, NULL},
        {ID_GLOBAL_VAR, NULL, NULL},
        {STRING, NULL, NULL}};

    size_t ARG_NAME_FIRST_LEN = sizeof(ARG_NAME_FIRST) / sizeof(ARG_NAME_FIRST[0]);

    if (nameHelperFunc(nextToken, ARG_NAME_FIRST, ARG_NAME_FIRST_LEN))
    {
        return 1;
    }
    program_error(file, 2, 4, *nextToken);
    /*int correctTokenType = 0;
    size_t i = 0;

    while (i < ARG_NAME_FIRST_LEN)
    {
        if (peek(&ARG_NAME_FIRST[i], *nextToken))
        {
            correctTokenType = 1;
        }
        if (correctTokenType == 1)
        {
            return 0;
        }
        i++;
    }
    program_error(file, 2, 4, *nextToken);*/
    return 0;
}

int VAR_NAME(TokenPtr *nextToken, FILE *file)
{
    static const target VAR_NAME_SEQ[] = {
        {IDENTIFIER, NULL, NULL},
        {ID_GLOBAL_VAR, NULL, NULL}};

    size_t VAR_NAME_SEQ_LEN = sizeof(VAR_NAME_SEQ) / sizeof(VAR_NAME_SEQ[0]);

    if (nameHelperFunc(nextToken, VAR_NAME_SEQ, VAR_NAME_SEQ_LEN))
    {
        return 1;
    }
    program_error(file, 2, 4, *nextToken);
    return 0;
}

int nameHelperFunc(TokenPtr *nextToken, const target *target, size_t target_len)
{
    size_t i = 0;
    while (i < target_len)
    {
        if (peek(&target[i], *nextToken))
        {
            return 1;
        }
        i++;
    }
    return 0;
}

// using strcmp compares target string with token. If token isn`t same as target string error is send to stderr output
/**
 * @brief checks if expected terminal and actual terminal are same
 * @param target expected terminal
 * @param token actual terminal
 *
 * @return wrong terminal will trigger stderr and program will end, correct terminal will return 0
 */
int peek(const target *target, TokenPtr token)
{
    if (target->type != token->type)
    {
        return 1;
    }
    else
    {
        if (target->data != NULL)
        {
            if (token->data == NULL)
            {
                fprintf(stderr, "SEGFAULT, token pointer is empty and you are trying reach something you can`t idiot\n");
                exit(2);
            }

            if (strcmp(target->data, token->data) != 0)
            {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

// helping function for more pleasing way to check matches and for updating nextToken
void for_function(const target *TARGE_SEQ, FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN)
{
    for (size_t i = 0; i < TARGE_SEQ_LEN; i++)
    {
        advance(&TARGE_SEQ[i], nextToken, file);
    }
}

/**
 * @brief check lookahead with function match and then iterate it
 * @return return next lookahead token
 */
void advance(const target *target, TokenPtr *token, FILE *file)
{
    if (peek(target, *token) != 0)
    {
        program_error(file, 2, 4, *token);
    }
    *token = lexer(file);
}
