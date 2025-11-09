//////////////////////////////////////////////
// filename: parser.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/psaParser.h"
#include "../include/stack.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*TODO:
 - create stack
 - create rules for PSA
 - implement AST node creation
 - implement PSA
*/

/**
 * @brief binary
 * E -> E + E
 * E -> E - E
 * E -> E * E
 * E -> E / E
 * @brief comp
 * E -> E == E
 * E -> E != E
 * E -> E < E
 * E -> E > E
 * E -> E <= E
 * E -> E >= E
 * @brief others
 * E -> (E)
 * E -> i
 * E -> E is Type
 */

char precedence_table[15][15] = {
    // *    /    +    -    ==   !=   <    >    <=   >=   (    )   id   is    $
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>', '>'}, // *
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>', '>'}, // /
    {'<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>', '>'}, // +
    {'<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>', '>'}, // -
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', '<', '>'}, // ==
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', '<', '>'}, // !=
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', ' ', '>'}, // <
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', ' ', '>'}, // >
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', ' ', '>'}, // <=
    {'<', '<', '<', '<', ' ', ' ', ' ', ' ', ' ', ' ', '<', '>', '<', ' ', '>'}, // >=
    {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=', '<', '<', ' '}, // (
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>', ' ', '>', '>'}, // )
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>', ' ', '>', '>'}, // i
    {' ', ' ', ' ', ' ', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', ' ', '>'}, // is
    {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', ' ', '<', '<', '$'}  // $
};

int parse_expression(TokenPtr *nextToken, FILE *file, target *endIfExp)
{
    stack_token stack;
    stack_token_init(&stack);

    static const TokenPtr endOfStackSymbol = {DOLLAR, "$", NULL};
    stack_token_push(&stack, &endOfStackSymbol);

    TokenPtr b = *nextToken;

    while (1)
    {
        TokenPtr a = stack_token_top(&stack);
    }
}

int converter(TokenPtr *tokenToConvert, FILE *file)
{
    TokenPtr token = (*tokenToConvert);
    if (token->type == "SPECIAL")
    {
        if (token->id == "(")
        {
            return LPAR;
        }
        else if (token->id == ")")
        {
            return RPAR;
        }
        program_error(file, 2, 4, tokenToConvert);
    }
    else if (token->type == "NUMERICAL")
    {
        return ID;
    }
    else if (token->type == "STRING")
    {
        return ID;
    }
    else if (token->type == "ARITHMETICAL")
    {
        if (token->id == "+")
        {
            return ADD;
        }
        else if (token->id == "-")
        {
            return SUB;
        }
        else if (token->id == "/")
        {
            return DIV;
        }
        else if (token->id == "*")
        {
            return MULL;
        }
    }
    else if (token->type == "CMP_OPERATOR")
    {
        if (token->id == "==")
        {
            return EQUAL;
        }
        else if (token->id == "!=")
        {
            return NEQUAL;
        }
        else if (token->id == "<")
        {
            return LESS;
        }
        else if (token->id == ">")
        {
            return GREATER;
        }
        else if (token->id == "<=")
        {
            return LSEQ;
        }
        else if (token->id == ">=")
        {
            return GREQ;
        }
    }
    else if (token->type == "KW_NULL_TYPE")
    {
        return ID;
    }
    else if (token->type == "KW_STRING")
    {
        return ID;
    }
    else if (token->type == "KW_NUM")
    {
        return ID;
    }
    else if (token->type == "KW_IS")
    {
        return IS;
    }
    program_error(file, 2, 4, token);
}