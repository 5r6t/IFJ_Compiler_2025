//////////////////////////////////////////////
// filename: psaParser.c                  	//
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//////////////////////////////////////////////

#include "parser.h"
#include "psaParser.h"
#include "stack.h"
#include "ast.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    {' ', ' ', ' ', ' ', '>', '>', ' ', ' ', ' ', ' ', ' ', ' ', '=', ' ', '>'}, // is
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
    {PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_GREATER, PRE_TAB_GREATER, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_NULL, PRE_TAB_EQUAL, PRE_TAB_NULL, PRE_TAB_GREATER},                              // is
    {PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL, PRE_TAB_LESS, PRE_TAB_LESS, PRE_TAB_NULL}                                         // $
};

static struct Token endOfStackSymbol = {DOLLAR, "$", NULL};
static struct Token shift = {SHIFT, "<", NULL};
static struct Token TOKEN_E = {E, "E", NULL};

/**
 * @brief Finds the nearest terminal symbol on the stack from the top.
 *
 * Scans the stack from the top downwards and returns the index of the first
 * terminal symbol it encounters. Non-terminal E and the SHIFT marker are
 * ignored during the scan.
 *
 * @param stack Pointer to the PSA stack structure.
 *
 * @return Index of the top-most terminal token on success, or -1 if no suitable
 *         terminal token is found or the stack is empty/invalid.
 */
int stack_token_top_terminal(stack_token *stack)
{

    if (!stack || stack->top < 0)
    {

        return -1;
    }

    for (int i = stack->top; i >= 0; i--)
    {
        TokenPtr token = stack->items[i].token;

        if (!token)
            continue;

        if (token->type == E || token->type == SHIFT)
            continue;

        return i;
    }
    return -1;
}

/**
 * @brief Parses an expression using precedence syntax analysis and builds its AST.
 *
 * Implements a precedence parser loop over the token stream starting from
 * @p *nextToken. The function:
 *  - Initializes a PSA stack with the end-of-stack symbol.
 *  - Repeatedly compares the top terminal on the stack (a) and current input
 *    token (b) using the precedence table.
 *  - According to the relation (=, <, >, or error) either:
 *      - pushes the input token,
 *      - inserts a SHIFT marker and then pushes the input token,
 *      - or performs a reduction via reduce().
 *  - Stops when the stack contains E just above DOLLAR and the lookahead token
 *    fulfills the end condition defined by @p endIfExp (peek()).
 *
 * On success, the AST node for the whole expression is returned and
 * @p *nextToken is updated to the first token after the expression.
 * On any syntax or internal error, program_error() is called.
 *
 * @param nextToken Pointer to the current token pointer; will be updated to the
 *                  first token following the parsed expression on success.
 * @param file      Source file handle used for lexical / syntax error reporting.
 * @param endIfExp  Pointer to a target structure describing the end-of-expression
 *                  condition (typically tokens that terminate an expression).
 *
 * @return ASTptr Root node of the parsed expression AST on success, or NULL if
 *         an error occurs (program_error() is invoked in that case).
 */
ASTptr parse_expression(TokenPtr *nextToken, FILE *file, const target *endIfExp)
{
    // fprintf(stderr,"som v parse expression\n");
    stack_token stack;
    stack_token_init(&stack); // kontrola ci stack sa inicializoval spravne

    stack_token_push(&stack, &endOfStackSymbol, NULL);

    TokenPtr b = *nextToken;

    while (1)
    {
        debug_stack(&stack);
        //  fprintf(stderr,"som vo while\n");
        stack_item top_item = stack_token_top(&stack);
        if (!top_item.token)
        {
            program_error(file, 2, 99, b);
            return NULL;
        }

        if (top_item.token->type == E && stack.top >= 1 && stack.items[stack.top - 1].token && stack.items[stack.top - 1].token->type == DOLLAR)
        {
            // fprintf(stderr,"dostal som sa do podmienky ukoncenia\n");
            if (peek(endIfExp, b, file))
            {
                ASTptr expressionNode = top_item.ast;
                *nextToken = b;
                // fprintf(stderr,"idem prec\n");
                return expressionNode;
            }
        }

        int a_index = stack_token_top_terminal(&stack);
        if (a_index < 0)
        {
            program_error(file, 2, 99, b);
            return NULL;
        }
        TokenPtr a = stack.items[a_index].token;

        int ia = converter(&a, file);
        fprintf(stderr, "converter vratil %d\n", ia);
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                b->type,
                b->id ? b->id : "NULL",
                b->data ? b->data : "NULL");
        int ib = converter(&b, file);
        fprintf(stderr, "converter vratil %d\n", ib);
        if (ia < 0 || ia >= PRECEDENCE_ROWS || ib < 0 || ib >= PRECEDENCE_COLS)
        {
            // fprintf(stderr,"som v chybe convertera\n");
            program_error(file, 2, 4, b);
            return NULL;
        }

        switch (precedence_table[ia][ib])
        {
        case PRE_TAB_EQUAL:
            fprintf(stderr, "=\n");
            stack_token_push(&stack, b, NULL);
            b = getToken(file);
            break;
        case PRE_TAB_LESS:
        {
            fprintf(stderr, "<\n");
            int a_index = stack_token_top_terminal(&stack);
            if (a_index < 0)
            {
                program_error(file, 2, 99, b);
                return NULL;
            }
            stack_token_insert_after(&stack, a_index, &shift, NULL);
            stack_token_push(&stack, b, NULL);
            b = getToken(file);
            break;
        }
        case PRE_TAB_GREATER:
            fprintf(stderr, ">\n");
            reduce(file, &stack);
            break;
        case PRE_TAB_NULL:
            fprintf(stderr, "NULL\n");
            program_error(file, 2, 4, b);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief Converts a token to an index in the precedence table.
 *
 * Maps the given token (operator, identifier-like symbol, parenthesis, etc.)
 * to a symbolic column/row index used in the precedence table for PSA.
 * Supported mappings include:
 *  - DOLLAR / NEWLINE    => END
 *  - '(' / ')'           => LPAR / RPAR
 *  - arithmetic ops      => ADD, SUB, DIV, MULL
 *  - comparison ops      => EQUAL, NEQUAL, LESS, GREATER, LSEQ, GREQ
 *  - identifiers, literals, type keywords, null => ID
 *  - keyword 'is'        => IS
 *
 * If the token is not supported or its combination (type / id) does not match
 * any known case, program_error() is called and -1 is returned.
 *
 * @param tokenToConvert Pointer to the token pointer that should be converted.
 * @param file           Source file handle used for error reporting.
 *
 * @return Integer index into the precedence table (e.g., END, ID, ADD, ...),
 *         or -1 if the token cannot be converted and program_error() is invoked.
 */
int converter(TokenPtr *tokenToConvert, FILE *file)
{
    TokenPtr token = (*tokenToConvert);

    if (token->type == DOLLAR)
    {
        return END;
    }
    else if (token->type == NEWLINE)
    {
        return END;
    }
    else if (token->type == SPECIAL)
    {
        if (token->id && strcmp(token->id, "(") == 0)
        {
            return LPAR;
        }
        else if (token->id && strcmp(token->id, ")") == 0)
        {
            return RPAR;
        }
        program_error(file, 2, 4, *tokenToConvert);
        return -1;
    }
    else if (token->type == NUMERICAL)
    {
        return ID;
    }
    else if (token->type == STRING)
    {
        return ID;
    }
    else if (token->type == ARITHMETICAL)
    {
        if (token->id && strcmp(token->id, "+") == 0)
        {
            return ADD;
        }
        else if (token->id && strcmp(token->id, "-") == 0)
        {
            return SUB;
        }
        else if (token->id && strcmp(token->id, "/") == 0)
        {
            return DIV;
        }
        else if (token->id && strcmp(token->id, "*") == 0)
        {
            return MULL;
        }
    }
    else if (token->type == CMP_OPERATOR)
    {
        if (token->id && strcmp(token->id, "==") == 0)
        {
            return EQUAL;
        }
        else if (token->id && strcmp(token->id, "!=") == 0)
        {
            return NEQUAL;
        }
        else if (token->id && strcmp(token->id, "<") == 0)
        {
            return LESS;
        }
        else if (token->id && strcmp(token->id, ">") == 0)
        {
            return GREATER;
        }
        else if (token->id && strcmp(token->id, "<=") == 0)
        {
            return LSEQ;
        }
        else if (token->id && strcmp(token->id, ">=") == 0)
        {
            return GREQ;
        }
    }
    else if (token->type && token->type == IDENTIFIER)
    {
        return ID;
    }
    else if (token->type && token->type == ID_GLOBAL_VAR)
    {
        return ID;
    }
    else if (token->type == KW_NULL_TYPE)
    {
        return ID;
    }
    else if (token->type == KW_STRING)
    {
        return ID;
    }
    else if (token->type == KW_NUM)
    {
        return ID;
    }
    else if (token->type == KW_IS)
    {
        return IS;
    }
    else if (token->type == KW_NULL)
    {
        return ID;
    }
    // fprintf(stderr,"nesedimi to\n");
    program_error(file, 2, 4, token);
    return -1;
}

/**
 * @brief Performs a single precedence-parser reduction on the PSA stack.
 *
 * Scans the expression stack from the top down to the nearest SHIFT marker
 * and determines the length of the handle to be reduced. According to the
 * handle length, it:
 *  - reduces a single terminal I (literal / identifier) to non-terminal E
 *    via checkForI(), or
 *  - reduces a three-symbol handle (E op E, "( E )", "E is Type") via
 *    checkForOtherRules().
 *
 * On success, the reduced symbols (including the SHIFT marker) are removed
 * from the stack and replaced with a single E carrying the constructed AST node.
 * On error, program_error() is called.
 *
 * @param file  Source file handle used for error reporting.
 * @param stack Pointer to the PSA stack of tokens and partial AST nodes.
 *
 * @return ASTptr Pointer to the AST node created by the reduction, or NULL if
 *         an error occurred or the caller should ignore the result and read it
 *         from the top of the stack instead.
 */
ASTptr reduce(FILE *file, stack_token *stack)
{
    fprintf(stderr, "som v reduce\n");
    int index = stack->top;
    int steps = 0;
    TokenPtr token = stack->items[index].token;

    while (token->type != SHIFT)
    {
        fprintf(stderr, "redukujem\n");
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                token->type,
                token->id ? token->id : "NULL",
                token->data ? token->data : "NULL");
        if (token->type == DOLLAR)
        {
            program_error(file, 2, 4, token);
        }
        steps++;
        index--;
        token = stack->items[index].token;
    }

    int index_shift = index;
    int reduce_len = steps;
    fprintf(stderr, "v reduce_len je  %d\n", reduce_len);
    if (reduce_len == 1)
        checkForI(index_shift, stack, file);
    else if (reduce_len == 3)
        checkForOtherRules(index_shift, stack, file);
    else
    {
        program_error(file, 2, 4, token);
        return NULL;
    }
    return NULL;
}

/**
 * @brief Reduces a single terminal symbol I to the non-terminal E.
 *
 * This rule handles terminals that directly correspond to literals or
 * identifiers (Num, String, null, local id, global id). It creates a new
 * AST node of type AST_LITERAL or AST_IDENTIFIER based on the token type.
 *
 * The function:
 *  - Reads the token immediately to the right of the SHIFT marker.
 *  - Allocates and fills an AST node.
 *  - Pops the handle including SHIFT from the stack.
 *  - Pushes a new stack item with non-terminal E and the created AST node.
 *
 * @param index_shift Index of the SHIFT marker within the stack.
 * @param stack       Pointer to the PSA stack.
 * @param file        Source file handle used for error reporting.
 *
 * @return ASTptr Pointer to the created AST node, or NULL if an error occurs
 *         (program_error() is called in that case).
 */
ASTptr checkForI(int index_shift, stack_token *stack, FILE *file)
{
    TokenPtr token = stack->items[index_shift + 1].token;
    fprintf(stderr, "som v checkForI");
    ASTptr node = NULL;
    node = malloc(sizeof(ASTnode));
    if (!node)
    {
        program_error(file, 99, 0, token);
    }

    if (token->type == NUMERICAL)
    {
        // fprintf(stderr,"som tu");
        node->type = AST_LITERAL;
        node->literal.liType = LIT_NUMBER;
        node->literal.num = strtod(token->data, NULL);
        // fprintf(stderr,"v node je ulozene %f", node->literal.num);
    }
    else if (token->type == STRING)
    {
        node->type = AST_LITERAL;
        node->literal.liType = LIT_STRING;
        node->literal.str = my_strdup(token->data);
    }
    else if (token->type == KW_NULL)
    {
        node->type = AST_LITERAL;
        node->literal.liType = LIT_NULL;
    }
    else if (token->type == IDENTIFIER)
    {
        node->type = AST_IDENTIFIER;
        node->identifier.idType = ID_LOCAL;
        node->identifier.name = my_strdup(token->id);
    }
    else if (token->type == ID_GLOBAL_VAR)
    {
        node->type = AST_IDENTIFIER;
        node->identifier.idType = ID_GLOBAL;
        node->identifier.name = my_strdup(token->id);
    }
    else
    {
        program_error(file, 2, 4, token);
        return NULL;
    }

    popRuleFromStack(index_shift, stack);
    stack_token_push(stack, &TOKEN_E, node);
    return node;
}

/**
 * @brief Reduces three-symbol handles of the form "( E )", "E op E" or "E is Type".
 *
 * Depending on the token following the SHIFT marker, this function chooses
 * the appropriate reduction rule:
 *
 *  - "( E )"         → calls ruleParenthesis()
 *  - "E <cmp> E"     → calls ruleComper()
 *  - "E <arith> E"   → calls ruleArithmetics()
 *  - "E is TypeName" → calls ruleIS()
 *
 * For each successful rule:
 *  - The corresponding AST node is created.
 *  - The whole handle including SHIFT is popped from the stack.
 *  - A new non-terminal E with the resulting AST node is pushed.
 *
 * If the pattern does not match any supported rule, program_error() is called.
 *
 * @param index Index of the SHIFT marker within the stack.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 *
 * @return ASTptr Pointer to the newly created AST node, or NULL on error.
 */
ASTptr checkForOtherRules(int index, stack_token *stack, FILE *file)
{
    // fprintf(stderr,"check for other rules\n");
    TokenPtr token = stack->items[index + 1].token;
    ASTptr node = NULL;
    if (token->type == SPECIAL && (strcmp(token->id, "(") == 0))
    {
        node = ruleParenthesis(index, stack, file);
        popRuleFromStack(index, stack);
        stack_token_push(stack, &TOKEN_E, node);
        return node;
    }
    else if (token->type != E)
    {
        // fprintf(stderr,"som vyhodeny\n");
        program_error(file, 2, 4, token);
        return NULL;
    }

    token = stack->items[index + 2].token;
    if (token->type == CMP_OPERATOR)
    {
        node = ruleComper(index, stack, file);
    }
    else if (token->type == ARITHMETICAL)
    {
        // fprintf(stderr,"som v arithmetics\n");
        node = ruleArithmetics(index, stack, file);
    }
    else if (token->type == KW_IS)
    {
        node = ruleIS(index, stack, file);
    }
    else
    {
        program_error(file, 2, 4, token);
        return NULL;
    }

    popRuleFromStack(index, stack);
    stack_token_push(stack, &TOKEN_E, node);
    return node;
}

/**
 * @brief Pops a complete reduction handle (including SHIFT) from the PSA stack.
 *
 * Removes all items from the top of the stack down to and including the item
 * at position @p index_shift. Used after a rule has been recognized and the
 * corresponding AST node has been created.
 *
 * @param index_shift Index of the first item (typically SHIFT) to be removed.
 * @param stack       Pointer to the PSA stack.
 */
void popRuleFromStack(int index_shift, stack_token *stack)
{
    for (int i = stack->top; i >= index_shift; i--)
    {
        stack_token_pop(stack);
    }
    return;
}

/**
 * @brief Reduction rule for parenthesized expressions "( E )".
 *
 * Expects a handle of the form:
 *   SHIFT '(' E ')'
 *
 * The function:
 *  - Verifies that the symbol after '(' is non-terminal E.
 *  - Verifies that the following symbol is a closing parenthesis ')'.
 *  - Returns the AST node stored with E without modifying the stack.
 *
 * Any mismatch leads to a syntax error via program_error().
 *
 * @param index Index of the SHIFT marker within the stack.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 *
 * @return ASTptr The AST node of the inner expression E, or NULL on error.
 */
ASTptr ruleParenthesis(int index, stack_token *stack, FILE *file)
{
    index += 2;
    TokenPtr token = stack->items[index].token;
    ASTptr node = NULL;
    if (token->type == E)
    {
        token = stack->items[index + 1].token;
        if (token->type == SPECIAL && (strcmp(token->id, ")") == 0))
        {
            node = stack->items[index].ast;
            return node;
        }
        program_error(file, 2, 4, token);
        return NULL;
    }
    program_error(file, 2, 4, token);
    return NULL;
}

/**
 * @brief Reduction rule for comparison operators "E <cmp> E".
 *
 * Expects a handle of the form:
 *   SHIFT E CMP_OPERATOR E
 *
 * The function:
 *  - Reads the left and right E AST nodes.
 *  - Maps the comparison operator token ("==", "!=", "<", "<=", ">", ">=")
 *    to the corresponding BinOpType value.
 *  - Verifies that the right symbol is non-terminal E via checkEnd().
 *  - Allocates an AST_BINOP node with the selected operator and both operands.
 *
 * On invalid operator or stack layout, program_error() is called.
 *
 * @param index Index of the SHIFT marker within the stack.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 *
 * @return ASTptr Pointer to the created AST_BINOP node, or NULL on error.
 */
ASTptr ruleComper(int index, stack_token *stack, FILE *file)
{
    ASTptr left = stack->items[index + 1].ast;
    int middle = index + 2;
    int end = index + 3;
    TokenPtr token = stack->items[middle].token;
    BinOpType op;
    ASTptr node = NULL;
    ASTptr right = NULL;
    if (strcmp(token->id, "==") == 0)
    {
        op = BINOP_EQ;
        checkEnd(end, stack, file);
    }
    else if (strcmp(token->id, "!=") == 0)
    {
        op = BINOP_NEQ;
        checkEnd(end, stack, file);
    }
    else if (strcmp(token->id, ">=") == 0)
    {
        op = BINOP_GTE;
        checkEnd(end, stack, file);
    }
    else if (strcmp(token->id, "<=") == 0)
    {
        op = BINOP_LTE;
        checkEnd(end, stack, file);
    }
    else if (strcmp(token->id, ">") == 0)
    {
        op = BINOP_GT;
        checkEnd(end, stack, file);
    }
    else if (strcmp(token->id, "<") == 0)
    {
        op = BINOP_LT;
        checkEnd(end, stack, file);
    }
    else
    {
        program_error(file, 2, 4, token);
        return NULL;
    }

    right = stack->items[end].ast;

    node = malloc(sizeof(ASTnode));
    node->type = AST_BINOP;
    node->binop.opType = op;
    node->binop.left = left;
    node->binop.right = right;

    return node;
}

/**
 * @brief Reduction rule for arithmetic operators "E + E", "E - E", "E * E", "E / E".
 *
 * Expects a handle of the form:
 *   SHIFT E ARITHMETICAL E
 *
 * The function:
 *  - Reads the left and right E AST nodes.
 *  - Maps the arithmetic operator token ("+", "-", "*", "/") to BinOpType.
 *  - Verifies that the right symbol is non-terminal E via checkEnd().
 *  - Allocates an AST_BINOP node with the selected operator and both operands.
 *
 * On invalid operator or stack layout, program_error() is called.
 *
 * @param index Index of the SHIFT marker within the stack.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 *
 * @return ASTptr Pointer to the created AST_BINOP node, or NULL on error.
 */
ASTptr ruleArithmetics(int index, stack_token *stack, FILE *file)
{
    // fprintf(stderr,"vosiel ruleArithmetics\n");
    ASTptr left = stack->items[index + 1].ast;
    int middle = index + 2;
    int end = index + 3;
    TokenPtr token = stack->items[middle].token;
    BinOpType op;
    ASTptr right;

    if (strcmp("*", token->id) == 0)
    {
        op = BINOP_MUL;
        checkEnd(end, stack, file);
    }
    else if (strcmp("-", token->id) == 0)
    {
        op = BINOP_SUB;
        checkEnd(end, stack, file);
    }
    else if (strcmp("+", token->id) == 0)
    {
        op = BINOP_ADD;
        checkEnd(end, stack, file);
    }
    else if (strcmp("/", token->id) == 0)
    {
        op = BINOP_DIV;
        checkEnd(end, stack, file);
    }
    else
    {
        program_error(file, 2, 4, token);
    }

    right = stack->items[end].ast;

    ASTptr node = malloc(sizeof(ASTnode));
    if (!node)
    {
        program_error(file, 99, 0, token);
        return NULL;
    }
    node->type = AST_BINOP;
    node->binop.opType = op;
    node->binop.left = left;
    node->binop.right = right;

    return node;
}

/**
 * @brief Reduction rule for the type test operator "E is TypeName".
 *
 * Expects a handle of the form:
 *   SHIFT E KW_IS TypeName
 *
 * The function:
 *  - Reads the left E AST node (expression being tested).
 *  - Maps the type keyword (Num, String, Null) to internal TypeName
 *    (TYPE_NUMBER, TYPE_STRING, TYPE_NULL).
 *  - Allocates an AST_BINOP node with operator BINOP_IS and the inferred
 *    result type stored in node->binop.resultType.
 *
 * The right AST child is set to NULL, because the type name is represented
 * directly by resultType, not by an expression subtree.
 *
 * On invalid type keyword, program_error() is called.
 *
 * @param index Index of the SHIFT marker within the stack.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 *
 * @return ASTptr Pointer to the created AST_BINOP node, or NULL on error.
 */
ASTptr ruleIS(int index, stack_token *stack, FILE *file)
{
    ASTptr left = stack->items[index + 1].ast;
    int end = index + 3;
    TokenPtr token = stack->items[end].token;

    ASTptr node = NULL;
    TypeName resType;
    ASTptr right = NULL;
    BinOpType op = BINOP_IS;

    if (token->type == KW_NULL_TYPE)
    {
        resType = TYPE_NULL;
    }
    else if (token->type == KW_NUM)
    {
        resType = TYPE_NUMBER;
    }
    else if (token->type == KW_STRING)
    {
        resType = TYPE_STRING;
    }
    else
    {
        program_error(file, 2, 4, token);
        return NULL;
    }

    node = malloc(sizeof(ASTnode));
    if (!node)
    {
        program_error(file, 99, 0, token);
        return NULL;
    }
    node->type = AST_BINOP;
    node->binop.opType = op;
    node->binop.resultType = resType;
    node->binop.left = left;
    node->binop.right = right;

    return node;
}

/**
 * @brief Verifies that the symbol at a given stack position is non-terminal E.
 *
 * Helper used by binary operator rules to ensure that the right-hand side of
 * a handle has already been reduced to non-terminal E. If the token at the
 * given index is not E, a syntax error is reported via program_error().
 *
 * @param end   Index of the symbol to check.
 * @param stack Pointer to the PSA stack.
 * @param file  Source file handle used for error reporting.
 */
void checkEnd(int end, stack_token *stack, FILE *file)
{
    TokenPtr token = stack->items[end].token;
    if (token->type != E)
    {
        program_error(file, 2, 4, token);
        return;
    }

    return;
}

// helper function for testing -> created by ai
void debug_stack(stack_token *stack)
{
    fprintf(stderr, "\n==== STACK DUMP (top=%d) ====\n", stack->top);

    for (int i = 0; i <= stack->top; i++)
    {
        stack_item it = stack->items[i];
        TokenPtr t = it.token;

        // token type
        int type = t ? t->type : -1;

        // token lexeme
        const char *lex =
            (t && t->id) ? t->id : (t && t->data) ? t->data
                                                  : "NULL";

        // AST flag
        const char *astinfo = (it.ast ? "AST" : "NULL");

        fprintf(stderr, "[%02d] type=%d  lex='%s'  ast=%s\n",
                i, type, lex, astinfo);
    }

    fprintf(stderr, "==== END STACK DUMP ====\n\n");
}