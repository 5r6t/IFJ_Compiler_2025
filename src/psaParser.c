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
        // node = malloc(sizeof(ASTnode));
        node->type = AST_LITERAL;
        node->literal.liType = LIT_NUMBER;
        node->literal.num = strtod(token->data, NULL);
        // fprintf(stderr,"v node je ulozene %f", node->literal.num);
    }
    else if (token->type == STRING)
    {
        // node = malloc(sizeof(ASTnode));
        node->type = AST_LITERAL;
        node->literal.liType = LIT_STRING;
        node->literal.str = my_strdup(token->data);
    }
    else if (token->type == KW_NULL)
    {
        // node = malloc(sizeof(ASTnode));
        node->type = AST_LITERAL;
        node->literal.liType = LIT_NULL;
    }
    else if (token->type == IDENTIFIER)
    {
        // node = malloc(sizeof(ASTnode));
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

    // left = (ASTptr)stack->items[index + 1]->data;
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

void popRuleFromStack(int index_shift, stack_token *stack)
{
    for (int i = stack->top; i >= index_shift; i--)
    {
        stack_token_pop(stack);
    }
    return;
}

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