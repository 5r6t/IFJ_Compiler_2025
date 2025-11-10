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
#include "../include/ast.h"

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
 * E -> id
 * E -> E is Type
 */

/*char precedence_table[15][15] = {
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
    {'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', ' ', '>', ' ', '>', '>'}, // id
    {' ', ' ', ' ', ' ', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '<', ' ', '>'}, // is
    {'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', ' ', '<', '<', '$'}  // $
};*/

PrecedentTableRel precedence_table[15][15] = {
    // *                   /                   +                   -                   ==                  !=                  <                   >                   <=                  >=                  (                   )                   id                  is                  $
    {PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER}, // *
    {PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER}, // /
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER},       // +
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_GREATER},       // -
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_GREATER},                                  // ==
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_GREATER},                                  // !=
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_GREATER},                                  // <
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_GREATER},                                  // >
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_GREATER},                                  // <=
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_GREATER, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_GREATER},                                  // >=
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_EQUAL, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL},                                       // (
    {PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_GREATER}, // )
    {PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_GREATER}, // id
    {PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_GREATER},                               // is
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL}                                         // $
};

ASTptr parse_expression(TokenPtr *nextToken, FILE *file, target *endIfExp)
{
    stack_token stack;
    stack_token_init(&stack);

    static const TokenPtr TOKEN_E = {E, "E", NULL};
    static const TokenPtr endOfStackSymbol = {DOLLAR, "$", NULL};
    static const TokenPtr shift = {SHIFT, "<", NULL};
    stack_token_push(&stack, &endOfStackSymbol);

    TokenPtr b = *nextToken;

    while (1)
    {
        TokenPtr a = stack_token_top(&stack);
        switch (precedence_table[converter(a, file)][converter(b, file)])
        {
        case PRE_TAB_EQUAL:
            stack_token_push(&stack, b);
            b = lexer(file);
            break;
        case PRE_TAB_LESS:
            stack_token_push(&stack, shift);
            stack_token_push(&stack, b);
            b = lexer(file);
            break;
        case PRE_TAB_GREATER:
            reduce(file, &stack);
            break;
        case PRE_TAB_NULL:
            program_error(file, 2, 4, b);
            break;
        default:
            break;
        }
        if (stack_token_top(&stack)->type == "E" && stack.items[stack.top - 1])
        {
            if (b == endIfExp)
            {
                return;
            }
            program_error(file, 2, 4, b);
        }
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

ASTptr reduce(FILE *file, stack_token *stack)
{
    int index = stack->top;
    int steps = 0;
    TokenPtr token = stack->items[index];

    while (token->type != "SHIFT")
    {
        if (token->type == "DOLLAR")
        {
            program_error(file, 2, 4, token);
        }
        steps++;
        index--;
        token = stack->items[index];
    }

    int index_shift = index;
    int reduce_len = steps;

    if (reduce_len == 1)
        checkForI(index_shift, &stack, file);
    else if (reduce_len == 3)
        checkForOtherRules(index_shift, &stack);
    else
    {
        program_error(file, 2, 4, token);
    }
    return;
}

ASTptr checkForI(int index_shift, stack_token *stack, FILE *file)
{
    static const TokenPtr TOKEN_E = {E, "E", NULL};
    TokenPtr token = stack->items[index_shift + 1];
    ASTptr node = NULL;
    if (token->type == ("NUMERICAL" || "STRING"))
    {
        node = malloc(sizeof(ASTnode));
        node->type = AST_LITERAL;
        node->literal.liType = LIT_NUMBER;
        // node->literal.num = token->data; -> musim zmenit na double;

        popRuleFromStack(index_shift);
        stack_token_push(&stack, TOKEN_E);
        return;
    }
    program_error(file, 2, 4, token);
}
