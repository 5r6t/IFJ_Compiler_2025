//////////////////////////////////////////////
// filename: parser.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jozef Matus (xmatusj00) / karfisk 	//
//////////////////////////////////////////////

#include "common.h"
#include "lex.h"
#include "parser.h"
#include "ast.h"
#include "psaParser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

TokenPtr lookahead;
bool pending = false;

/* LL1:

PROGRAM ::= PROLOG CLASS
PROLOG ::= import "ifj25" for Ifj /
CLASS ::= class Program { / FUNCTIONS }
FUNCTIONS ::= static FUNC_NAME FUNC_GET_SET_DEF FUNCTIONS
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

/**
 * @brief Entry point of the syntactic analysis.
 *
 * Initializes the lexical input by reading the first token, skips leading
 * newlines, and calls PROGRAM() to parse the whole source as a program.
 * After PROGRAM() returns, it expects FILE_END; otherwise, a syntax error
 * is reported via program_error().
 *
 * @param file Source file handle used by the lexer and for error reporting.
 *
 * @return Root AST node representing the whole program.
 */
ASTptr parser(FILE *file)
{
    TokenPtr token = getToken(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    skip_newline(file, &token);
    ASTptr root = PROGRAM(&token, file);
    token = getToken(file);
    if (token->type != FILE_END)
    {
        program_error(file, 2, 4, token);
    }
    fprintf(stderr, "Syntakticka analyza prebehla uspesne!!!\n");
    return root;
}

/**
 * @brief Parses the start symbol PROGRAM of the LL grammar.
 *
 * Implements the rule:
 *   PROGRAM ::= PROLOG CLASS
 *
 * First parses the mandatory PROLOG (import of the ifj25 library),
 * and then parses the single CLASS definition, which becomes the root
 * AST node.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return AST node representing the program (class Program with its functions).
 */
ASTptr PROGRAM(TokenPtr *nextToken, FILE *file)
{
    fprintf(stderr, "som v programe\n");
    PROLOG(nextToken, file);

    ASTptr program = (ASTptr)malloc(sizeof(ASTnode));

    program = CLASS(nextToken, file);

    return program;
}

/**
 * @brief Parses the mandatory PROLOG of the source file.
 *
 * Implements the rule:
 *   PROLOG ::= import "ifj25" for Ifj /
 *
 * The function verifies the exact sequence:
 *   import "ifj25" for Ifj NEWLINE
 *
 * Any deviation results in a syntax error.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 */
void PROLOG(TokenPtr *nextToken, FILE *file)
{
    static const target PROLOG_START = {KW_IMPORT, NULL, "import"};
    static const target PROLOG_LAN = {STRING, "ifj25", NULL};
    static const target PROLOG_FOR = {KW_FOR, NULL, "for"};
    static const target PROLOG_IFJ = {KW_IFJ, NULL, "Ifj"};
    static const target PROLOG_NEWLINE = {NEWLINE, NULL, NULL};

    advance(&PROLOG_START, nextToken, file);
    skip_newline(file, nextToken);

    advance(&PROLOG_LAN, nextToken, file);
    skip_newline(file, nextToken);

    advance(&PROLOG_FOR, nextToken, file);
    skip_newline(file, nextToken);

    advance(&PROLOG_IFJ, nextToken, file);

    advance(&PROLOG_NEWLINE, nextToken, file);

    return;
}

/**
 * @brief Parses the main class definition.
 *
 * Implements the rule:
 *   CLASS ::= class Program { / FUNCTIONS }
 *
 * Checks the fixed prolog of the class using for_function(), allocates and
 * initializes an AST_PROGRAM node, and then calls FUNCTIONS() to parse all
 * contained static function definitions. Finally consumes the closing '}'.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return AST node of type AST_PROGRAM containing an array of function nodes.
 */
ASTptr CLASS(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    static const target CLASS_TARGET[] =
        {
            {KW_CLASS, NULL, "class"},
            {IDENTIFIER, NULL, "Program"},
            {SPECIAL, NULL, "{"},
            {NEWLINE, NULL, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(CLASS_TARGET) / sizeof(CLASS_TARGET[0]);
    for_function(CLASS_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    ASTptr class = (ASTptr)malloc(sizeof(struct ASTnode));
    if (!class)
    {
        program_error(file, 0, 0, *nextToken);
    }
    class->type = AST_PROGRAM;
    class->program.funcs = NULL;
    class->program.funcsCount = 0;
    class->program.funcsCap = 0;

    FUNCTIONS(nextToken, file, class);

    static const target CLASS_TARGET_END = {SPECIAL, NULL, "}"};
    advance(&CLASS_TARGET_END, nextToken, file);

    return class;
}

/**
 * @brief Parses a (possibly empty) sequence of static function definitions.
 *
 * Implements the rules:
 *   FUNCTIONS ::= static FUNC_NAME FUNC_GET_SET_DEF FUNCTIONS
 *   FUNCTIONS ::= ''
 *
 * When the current token is 'static', it:
 *   - consumes the keyword,
 *   - parses the function name via FUNC_NAME(),
 *   - allocates and initializes an AST_FUNC_DEF node,
 *   - parses the function header/body via FUNC_GET_SET_DEF(),
 *   - appends the function node to the program's function array
 *     (reallocating if needed),
 *   - and then recurses to parse further FUNCTIONS().
 *
 * When the current token matches the follow symbol '}', it represents epsilon
 * and the function simply returns the existing program node.
 *
 * Any other token triggers a syntax error.
 *
 * @param nextToken   Pointer to the current token pointer.
 * @param file        Source file handle used for error reporting.
 * @param programNode AST node of type AST_PROGRAM to which functions are added.
 *
 * @return The AST_PROGRAM node passed in @p programNode, for convenience.
 */
ASTptr FUNCTIONS(TokenPtr *nextToken, FILE *file, ASTptr programNode) // change return type to ASTnode
{
    static const target FUNCTIONS_FOLLOW = {SPECIAL, NULL, "}"};

    skip_newline(file, nextToken);

    if ((*nextToken)->type == KW_STATIC)
    {
        static const target FUNCTIONS_FIRST = {KW_STATIC, NULL, "static"};
        advance(&FUNCTIONS_FIRST, nextToken, file);

        TokenPtr funcName = *nextToken;

        if (FUNC_NAME(nextToken, file))
        {
            program_error(file, 2, 4, *nextToken);
        }

        ASTptr function = (ASTptr)malloc(sizeof(ASTnode));
        function->type = AST_FUNC_DEF;
        function->func.name = my_strdup(funcName->id);
        function->func.paramNames = NULL;
        function->func.paramCount = 0;
        function->func.body = NULL;
        function->func.isGetter = false;
        function->func.isSetter = false;

        FUNC_GET_SET_DEF(nextToken, file, function);

        // check if realloc is needed
        if (programNode->program.funcsCount == programNode->program.funcsCap)
        {
            int newCap;
            if (programNode->program.funcsCap == 0)
            {
                newCap = 4;
            }
            else
            {
                newCap = programNode->program.funcsCap * 2;
            }

            // realloc programNode
            ASTptr *newProgramNode = realloc(programNode->program.funcs, newCap * sizeof(ASTptr));
            if (!newProgramNode)
            {
                program_error(file, 0, 0, *nextToken);
            }

            programNode->program.funcsCap = newCap;
            programNode->program.funcs = newProgramNode;
        }

        // add to function to dynamic array
        programNode->program.funcs[programNode->program.funcsCount] = function;
        programNode->program.funcsCount++;

        return FUNCTIONS(nextToken, file, programNode);
    }
    else if (peek(&FUNCTIONS_FOLLOW, *nextToken, file)) // epsilon
    {
        return programNode;
    }
    else // should call program error
    {
        program_error(file, 2, 4, *nextToken);
    }
    return NULL;
}

/**
 * @brief Parses a function name according to the LL grammar.
 *
 * Currently accepts only IDENTIFIER as a function name and consumes it
 * from the input by calling getToken().
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return 0 on success (identifier found and consumed), non-zero on failure.
 *         On failure, program_error() is called.
 */
int FUNC_NAME(TokenPtr *nextToken, FILE *file)
{
    if ((*nextToken)->type == IDENTIFIER)
    {
        *nextToken = getToken(file);
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

/**
 * @brief Parses function definition, getter, or setter body and header details.
 *
 * Implements the rules:
 *   FUNC_GET_SET_DEF ::= ( PAR ) { / FUNC_BODY } /
 *   FUNC_GET_SET_DEF ::= { / FUNC_BODY } /
 *   FUNC_GET_SET_DEF ::= = ( id ) { / FUNC_BODY } /
 *
 * The function:
 *   - For a standard function '(...){ ... }':
 *       * parses parameter list via PAR(),
 *       * parses the opening of the body with for_function(),
 *       * builds an AST block node for the body via FUNC_BODY().
 *   - For a getter '{ ... }':
 *       * marks the function as getter,
 *       * parses the body starting after '{' and NEWLINE.
 *   - For a setter '=(id){ ... }':
 *       * marks the function as setter,
 *       * takes one identifier parameter,
 *       * fills functionNode->func.paramNames accordingly,
 *       * parses the body similarly to a normal function.
 *
 * In all cases, the function verifies the final sequence "} NEWLINE" using
 * for_function(). Any mismatch results in a syntax error.
 *
 * @param nextToken    Pointer to the current token pointer.
 * @param file         Source file handle used for error reporting.
 * @param functionNode AST_FUNC_DEF node that is being completed.
 *
 * @return The same @p functionNode pointer after it has been filled.
 */
ASTptr FUNC_GET_SET_DEF(TokenPtr *nextToken, FILE *file, ASTptr functionNode)
{
    static target FUNC_DEF = {SPECIAL, NULL, "("};

    static target FUNC_GET = {SPECIAL, NULL, "{"};

    static target FUNC_IDENT = {IDENTIFIER, NULL, NULL};

    static const target FUNC_SET_SEQ[] =
        {
            {SPECIAL, NULL, "="},
            {SPECIAL, NULL, "("},
        };

    target FUNC_DEF_SEQ[] =
        {
            {SPECIAL, NULL, ")"},
            {SPECIAL, NULL, "{"},
            {NEWLINE, NULL, NULL}};

    target FUNC_GET_SET_DEF_END[] = // last two lexem in rules for FUNC_GET_SET_DEF are same -> } EOL
        {
            {SPECIAL, NULL, "}"},
            {NEWLINE, NULL, NULL}};

    size_t FUNC_SET_SEQ_LEN = sizeof(FUNC_SET_SEQ) / sizeof(FUNC_SET_SEQ[0]);
    size_t FUNC_DEF_SEQ_LEN = sizeof(FUNC_DEF_SEQ) / sizeof(FUNC_DEF_SEQ[0]);
    size_t FUNC_GET_SET_DEF_END_LEN = sizeof(FUNC_GET_SET_DEF_END) / sizeof(FUNC_GET_SET_DEF_END[0]);

    if (peek(&FUNC_DEF, *nextToken, file)) // definition of function
    {
        *nextToken = getToken(file);

        // Loading par of function into the AST
        parArr pA;
        parArrInit(&pA);
        PAR(nextToken, file, &pA);
        functionNode->func.paramNames = pA.parNames;
        functionNode->func.paramCount = pA.arrCnt;
        for_function(FUNC_DEF_SEQ, file, nextToken, FUNC_DEF_SEQ_LEN);

        // Loading function body to the AST
        ASTptr blockNode = (ASTptr)malloc(sizeof(ASTnode));
        if (!blockNode)
        {
            program_error(file, 0, 0, *nextToken);
        }
        blockNodeInit(blockNode);
        FUNC_BODY(nextToken, file, blockNode);
        fprintf(stderr, "vratil som sa z body\n");
        functionNode->func.body = blockNode;

        fprintf(stderr, "idem na koniec toto je v tokene\n");
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return functionNode;
    }
    else if (peek(&FUNC_GET, *nextToken, file)) // getter
    {
        *nextToken = getToken(file);
        static target AFTER_BRACE = {NEWLINE, NULL, NULL};

        functionNode->func.isGetter = true;
        functionNode->func.isSetter = false;

        advance(&AFTER_BRACE, nextToken, file);

        ASTptr blockNode = (ASTptr)malloc(sizeof(ASTnode));
        if (!blockNode)
        {
            program_error(file, 0, 0, *nextToken);
        }
        blockNodeInit(blockNode);
        FUNC_BODY(nextToken, file, blockNode);
        functionNode->func.body = blockNode;

        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return functionNode;
    }
    else if ((*nextToken)->type == SPECIAL && strcmp((*nextToken)->id, "=") == 0) // setter
    {
        for_function(FUNC_SET_SEQ, file, nextToken, FUNC_SET_SEQ_LEN);

        // IDENTIFIER
        char *name = my_strdup((*nextToken)->id);
        advance(&FUNC_IDENT, nextToken, file);

        // AST
        functionNode->func.isSetter = true;
        functionNode->func.isGetter = false;

        functionNode->func.paramCount = 1;
        functionNode->func.paramNames = malloc(sizeof(char *));
        if (!functionNode->func.paramNames)
        {
            program_error(file, 0, 0, *nextToken);
        }

        functionNode->func.paramNames[0] = name;

        // continue
        for_function(FUNC_DEF_SEQ, file, nextToken, FUNC_DEF_SEQ_LEN);
        ASTptr blockNode = (ASTptr)malloc(sizeof(ASTnode));
        if (!blockNode)
        {
            program_error(file, 0, 0, *nextToken);
        }
        blockNodeInit(blockNode);
        FUNC_BODY(nextToken, file, blockNode);
        functionNode->func.body = blockNode;
        for_function(FUNC_GET_SET_DEF_END, file, nextToken, FUNC_GET_SET_DEF_END_LEN);
        return functionNode;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }

    return NULL;
}

/**
 * @brief Parses the parameter list of a function.
 *
 * Implements the rules:
 *   PAR      ::= '' | id NEXT_PAR
 *   NEXT_PAR ::= , id NEXT_PAR | ''
 *
 * Adds all parameter names into the provided parArr structure.
 *
 * @param nextToken Pointer to the current token pointer (starting after '(').
 * @param file      Source file handle used for error reporting.
 * @param pA        Pointer to a parameter array structure to be filled.
 */
void PAR(TokenPtr *nextToken, FILE *file, parArr *pA)
{
    static const target PAR_FIRST = {IDENTIFIER, NULL, NULL};
    static const target PAR_FOLLOW = {SPECIAL, NULL, ")"};
    if ((*nextToken)->type == IDENTIFIER)
    {
        char *name = my_strdup((*nextToken)->id);
        parArrAdd(pA, name, file, *nextToken);

        advance(&PAR_FIRST, nextToken, file);

        NEXT_PAR(nextToken, file, pA);

        return;
    }
    else if (peek(&PAR_FOLLOW, *nextToken, file))
    {
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return;
}

/**
 * @brief Parses the continuation of a parameter list.
 *
 * Implements the rule:
 *   NEXT_PAR ::= , id NEXT_PAR | ''
 *
 * On ',', consumes the comma, reads the following identifier, adds it into
 * @p pA, and recurses. If the current symbol is ')', the rule is epsilon.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 * @param pA        Pointer to a parameter array structure to be filled.
 */
void NEXT_PAR(TokenPtr *nextToken, FILE *file, parArr *pA)
{
    static const target NEXT_PAR_FIRST = {SPECIAL, NULL, ","};
    static const target NEXT_PAR_FOLLOW = {SPECIAL, NULL, ")"};
    static const target NEXT_PAR_IDEN = {IDENTIFIER, NULL, NULL};
    if (peek(&NEXT_PAR_FIRST, *nextToken, file))
    {
        advance(&NEXT_PAR_FIRST, nextToken, file);

        char *name = my_strdup((*nextToken)->id);
        parArrAdd(pA, name, file, *nextToken);
        advance(&NEXT_PAR_IDEN, nextToken, file);

        NEXT_PAR(nextToken, file, pA);

        return;
    }
    else if (peek(&NEXT_PAR_FOLLOW, *nextToken, file)) // epsilon
    {
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return;
}

/**
 * @brief Parses the body of a function as a sequence of statements.
 *
 * Implements the rules:
 *   FUNC_BODY ::= ''
 *               | VAR_DECL FUNC_BODY
 *               | VAR_ASS_CALL_GET FUNC_BODY
 *               | IF_STAT FUNC_BODY
 *               | WHILE FUNC_BODY
 *               | RETURN FUNC_BODY
 *               | { / FUNC_BODY } / FUNC_BODY
 *               | Ifj . id ( ARG ) / FUNC_BODY
 *
 * The function dispatches to different constructs based on the current token:
 *   - variable declarations (var),
 *   - assignments and local/global variable operations,
 *   - if/else statements,
 *   - while loops,
 *   - return statements,
 *   - nested blocks,
 *   - built-in function calls (Ifj.xxx),
 *   - epsilon when encountering the closing '}' of the surrounding block.
 *
 * Statements are appended into the given block AST node via varNameAdd().
 * Expression parsing within conditions and right-hand sides is delegated to
 * parse_expression() and RSA().
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 * @param blockNode AST node of type block that accumulates statements.
 *
 * @return The same @p blockNode pointer when the rule finishes (epsilon).
 */
ASTptr FUNC_BODY(TokenPtr *nextToken, FILE *file, ASTptr blockNode)
{

    static const target END_IF_EXP = {SPECIAL, NULL, ")"};
    static const target RETURN_FIRST = {KW_RETURN, NULL, NULL};
    static const target FUNC_BODY_END = {NEWLINE, NULL, NULL};
    static const target FUNC_BODY_FOLLOW = {SPECIAL, NULL, "}"};
    static const target VAR_ASS_CALL_GET = {SPECIAL, NULL, "="};
    static const target END_RETURN_EXP = {NEWLINE, NULL, NULL};
    static const target START_BLOCK = {SPECIAL, NULL, "{"};
    static const target BLOCK_NEWLINE = {NEWLINE, NULL, NULL};
    static const target DOT_INBUILD = {SPECIAL, NULL, "."};
    static const target ID_INBUILD = {IDENTIFIER, NULL, NULL};
    static const target PARAN_INBUILD = {SPECIAL, NULL, "("};

    static const target IF_STATMENT_START_SEQ[] =
        {
            {KW_IF, NULL, "if"},
            {SPECIAL, NULL, "("}};

    static const target IF_STATMENT_MIDDLE_SEQ[] =
        {
            {SPECIAL, NULL, ")"},
            {SPECIAL, NULL, "{"},
            {NEWLINE, NULL, NULL}};

    static const target IF_STATMENT_ELSE_BRANCH_SEQ[] =
        {
            {SPECIAL, NULL, "}"},
            {KW_ELSE, NULL, NULL},
            {SPECIAL, NULL, "{"},
            {NEWLINE, NULL, NULL}};

    static const target END_SEQ[] =
        {
            {SPECIAL, NULL, "}"},
            {NEWLINE, NULL, NULL}};

    static const target END_INBUILD_SEQ[] =
        {
            {SPECIAL, NULL, ")"},
            {NEWLINE, NULL, NULL}};

    static const target WHILE_START_SEQ[] =
        {
            {KW_WHILE, NULL, NULL},
            {SPECIAL, NULL, "("}};

    size_t END_INBUILD_SEQ_LEN = sizeof(END_INBUILD_SEQ) / sizeof(END_INBUILD_SEQ[0]);
    size_t IF_STATMENT_ELSE_BRANCH_SEQ_LEN = sizeof(IF_STATMENT_ELSE_BRANCH_SEQ) / sizeof(IF_STATMENT_ELSE_BRANCH_SEQ[0]);
    size_t IF_STATMENT_MIDDLE_SEQ_LEN = sizeof(IF_STATMENT_MIDDLE_SEQ) / sizeof(IF_STATMENT_MIDDLE_SEQ[0]);
    size_t IF_STATMENT_START_SEQ_LEN = sizeof(IF_STATMENT_START_SEQ) / sizeof(IF_STATMENT_START_SEQ[0]);
    size_t WHILE_START_SEQ_LEN = sizeof(WHILE_START_SEQ) / sizeof(WHILE_START_SEQ[0]);
    size_t END_SEQ_LEN = sizeof(END_SEQ) / sizeof(END_SEQ[0]);

    fprintf(stderr, "idem do body\n");
    skip_newline(file, nextToken);
    if ((*nextToken)->type == KW_VAR) // declare
    {
        fprintf(stderr, "deklaracia\n");
        *nextToken = getToken(file);
        char *varName = (*nextToken)->id;
        int result = VAR_NAME(nextToken, file);
        if (!result)
            program_error(file, 2, 4, *nextToken);

        ASTptr varNode = (ASTptr)malloc(sizeof(ASTnode));
        varNode->type = AST_VAR_DECL;
        varNode->var_decl.varName = my_strdup(varName);

        *nextToken = getToken(file);
        advance(&FUNC_BODY_END, nextToken, file);

        varNameAdd(blockNode, varNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == IDENTIFIER) // assign
    {
        char *varName = (*nextToken)->id;
        (*nextToken) = getToken(file);
        ASSIGN_OR_CALL(nextToken, file, blockNode, varName);
        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == ID_GLOBAL_VAR)
    {
        char *varName = (*nextToken)->id;
        ASTptr assignNode = (ASTptr)malloc(sizeof(ASTnode));
        assignNode->type = AST_ASSIGN_STMT;
        assignNode->assign_stmt.targetName = my_strdup(varName);
        assignNode->assign_stmt.asType = TARGET_GLOBAL;

        *nextToken = getToken(file);
        advance(&VAR_ASS_CALL_GET, nextToken, file);
        ASTptr exprNode = RSA(nextToken, file);

        assignNode->assign_stmt.expr = exprNode;

        varNameAdd(blockNode, assignNode, file, *nextToken);
        fprintf(stderr, "vraciam sa z body\n");
        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_IF) // if statement
    {
        fprintf(stderr, "if\n");
        ASTptr ifNode = (ASTptr)malloc(sizeof(ASTnode));
        ifNode->type = AST_IF_STMT;

        for_function(IF_STATMENT_START_SEQ, file, nextToken, IF_STATMENT_START_SEQ_LEN);

        ASTptr condition = parse_expression(nextToken, file, &END_IF_EXP);
        ifNode->ifstmt.cond = condition;

        for_function(IF_STATMENT_MIDDLE_SEQ, file, nextToken, IF_STATMENT_MIDDLE_SEQ_LEN);

        ASTptr thenBlock = (ASTptr)malloc(sizeof(ASTnode));
        blockNodeInit(thenBlock);
        FUNC_BODY(nextToken, file, thenBlock);
        ifNode->ifstmt.then = thenBlock;

        for_function(IF_STATMENT_ELSE_BRANCH_SEQ, file, nextToken, IF_STATMENT_ELSE_BRANCH_SEQ_LEN);

        ASTptr elseBlock = (ASTptr)malloc(sizeof(ASTnode));
        blockNodeInit(elseBlock);
        FUNC_BODY(nextToken, file, elseBlock);
        ifNode->ifstmt.elsestmt = elseBlock;

        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);

        varNameAdd(blockNode, ifNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_WHILE) // while statement
    {
        fprintf(stderr, "while\n");
        ASTptr whileNode = (ASTptr)malloc(sizeof(ASTnode));
        whileNode->type = AST_WHILE_STMT;

        for_function(WHILE_START_SEQ, file, nextToken, WHILE_START_SEQ_LEN);

        ASTptr condition = parse_expression(nextToken, file, &END_IF_EXP);
        whileNode->while_stmt.cond = condition;

        for_function(IF_STATMENT_MIDDLE_SEQ, file, nextToken, IF_STATMENT_MIDDLE_SEQ_LEN);

        ASTptr whileBody = (ASTptr)malloc(sizeof(ASTnode));
        blockNodeInit(whileBody);
        FUNC_BODY(nextToken, file, whileBody);
        whileNode->while_stmt.body = whileBody;

        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);

        varNameAdd(blockNode, whileNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_RETURN) // return
    {
        fprintf(stderr, "return\n");

        ASTptr returnNode = (ASTptr)malloc(sizeof(ASTnode));
        returnNode->type = AST_RETURN_STMT;
        returnNode->return_stmt.expr = NULL;

        advance(&RETURN_FIRST, nextToken, file);
        // ----------------- EXTENTION EXTSTAT ------------------
        /* if ((*nextToken)->type == NEWLINE)
        {
            advance(&FUNC_BODY_END, nextToken, file);
        }
        else
        {
            ASTptr condition = parse_expression(nextToken, file, &END_RETURN_EXP);
            returnNode->return_stmt.expr = condition;
            advance(&FUNC_BODY_END, nextToken, file);
        } */

        ASTptr condition = parse_expression(nextToken, file, &END_RETURN_EXP);
        returnNode->return_stmt.expr = condition;
        advance(&FUNC_BODY_END, nextToken, file);

        varNameAdd(blockNode, returnNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if (peek(&START_BLOCK, *nextToken, file))
    {
        (*nextToken) = getToken(file);
        advance(&BLOCK_NEWLINE, nextToken, file);
        ASTptr innerBlock = (ASTptr)malloc(sizeof(ASTnode));
        if (!innerBlock)
        {
            program_error(file, 0, 0, *nextToken);
        }
        blockNodeInit(innerBlock);
        FUNC_BODY(nextToken, file, innerBlock);
        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        varNameAdd(blockNode, innerBlock, file, *nextToken);
        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_IFJ)
    {
        fprintf(stderr, "INBUILD\n");
        (*nextToken) = getToken(file);
        advance(&DOT_INBUILD, nextToken, file);
        const char *prefix = "Ifj.";
        char *varName = (*nextToken)->id;
        advance(&ID_INBUILD, nextToken, file);

        size_t len = strlen(prefix) + strlen(varName) + 1;
        char *functionName = malloc(len);
        if (!functionName)
        {
            program_error(file, 0, 0, *nextToken);
        }

        strcpy(functionName, prefix);
        strcat(functionName, varName);

        // call initialize
        ASTptr inbuildCallNode = (ASTptr)malloc(sizeof(ASTnode));
        inbuildCallNode->type = AST_FUNC_CALL;
        inbuildCallNode->call.funcName = functionName;
        inbuildCallNode->call.argCap = 0;
        inbuildCallNode->call.argCount = 0;
        inbuildCallNode->call.callInfo = NULL;
        advance(&PARAN_INBUILD, nextToken, file);

        ArgArr argArr;
        argArrInit(&argArr);
        ARG(nextToken, file, &argArr);

        inbuildCallNode->call.argCount = argArr.arrCnt;
        inbuildCallNode->call.argCap = argArr.arrCnt;
        inbuildCallNode->call.args = argArr.items;

        varNameAdd(blockNode, inbuildCallNode, file, *nextToken);
        for_function(END_INBUILD_SEQ, file, nextToken, END_INBUILD_SEQ_LEN);
        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if (peek(&FUNC_BODY_FOLLOW, *nextToken, file)) // epsilon
    {
        fprintf(stderr, "som v epsilone\n");
        return blockNode;
    }
    else
    {
        fprintf(stderr, "skoncil som v chybe pre FUNC_BODY\n");
        program_error(file, 2, 4, *nextToken);
    }
    fprintf(stderr, "dostal som sa sem?\n");
    return NULL;
}

/**
 * @brief Distinguishes between an assignment and a function call starting with an identifier.
 *
 * Implements the rule:
 *   VAR_ASS_CALL_GET ::= VAR_NAME = RSA
 *                      | VAR_NAME ( ARG ) /
 *
 * Using lookahead, it decides whether the identifier is followed by '='
 * (assignment) or '(' (function call). In the assignment case, it constructs
 * an AST_ASSIGN_STMT node; in the call case, it constructs an AST_FUNC_CALL
 * node with parsed arguments.
 *
 * The created statement is appended to the given block using varNameAdd().
 *
 * @param nextToken Pointer to the current token pointer (already after VAR_NAME).
 * @param file      Source file handle used for error reporting.
 * @param blockNode Current block node where the statement is added.
 * @param varName   Name of the variable or function (identifier) just read.
 */
void ASSIGN_OR_CALL(TokenPtr *nextToken, FILE *file, ASTptr blockNode, char *varName)
{
    static const target ASSIGN_START = {SPECIAL, NULL, "="};
    static const target CALL_START = {SPECIAL, NULL, "("};
    if (peek(&ASSIGN_START, *nextToken, file))
    {
        fprintf(stderr, "priradenie\n");

        ASTptr assignNode = (ASTptr)malloc(sizeof(ASTnode));
        assignNode->type = AST_ASSIGN_STMT;
        assignNode->assign_stmt.targetName = my_strdup(varName);
        assignNode->assign_stmt.asType = TARGET_LOCAL;

        *nextToken = getToken(file);
        ASTptr exprNode = RSA(nextToken, file);

        assignNode->assign_stmt.expr = exprNode;

        varNameAdd(blockNode, assignNode, file, *nextToken);
        fprintf(stderr, "vraciam sa z ASSIGN\n");
        return;
    }
    else if (peek(&CALL_START, *nextToken, file))
    {
        fprintf(stderr, "call\n");
        ASTptr callNode = (ASTptr)malloc(sizeof(ASTnode));
        callNode->type = AST_FUNC_CALL;
        callNode->call.funcName = my_strdup(varName);
        callNode->call.argCap = 0;
        callNode->call.argCount = 0;
        callNode->call.args = NULL;
        callNode->call.callInfo = NULL;

        ArgArr argArr;
        argArrInit(&argArr);

        *nextToken = getToken(file);
        ARG(nextToken, file, &argArr);

        callNode->call.argCap = argArr.arrCap;
        callNode->call.argCount = argArr.arrCnt;
        callNode->call.args = argArr.items;

        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");

        if ((*nextToken)->type != NEWLINE)
        {
            fprintf(stderr, "nemam newline\n");
            program_error(file, 2, 4, *nextToken);
        }
        (*nextToken) = getToken(file);

        varNameAdd(blockNode, callNode, file, *nextToken);
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
        return;
    }
}

/**
 * @brief Parses the right-hand side of an assignment (RSA).
 *
 * Implements the grammar fragment:
 *   RSA ::= EXPRESSION /
 *         | id FUNC_TYPE /
 *         | Ifj . id ( ARG ) /
 *
 * The function:
 *   - Recognizes built-in calls of the form Ifj.name(args) and creates
 *     an AST_FUNC_CALL node.
 *   - For numeric, string, null literals or parenthesized expressions,
 *     calls parse_expression() and returns the resulting expression AST.
 *   - For plain identifiers / global variables, uses lookahead to distinguish
 *     between a function call and a general expression, and dispatches
 *     accordingly.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return AST node representing the parsed expression or function call.
 */
ASTptr RSA(TokenPtr *nextToken, FILE *file)
{
    static const target END_SEQ[] =
        {
            {SPECIAL, NULL, ")"},
            {NEWLINE, NULL, NULL}};

    static const target END_TARGET = {NEWLINE, NULL, NULL};
    static const target DOT_INBUILD = {SPECIAL, NULL, "."};
    static const target ID_INBUILD = {IDENTIFIER, NULL, NULL};
    static const target PARAN_INBUILD = {SPECIAL, NULL, "("};

    size_t END_SEQ_LEN = sizeof(END_SEQ) / sizeof(END_SEQ[0]);

    fprintf(stderr, "som v RSA\n");
    if ((*nextToken)->type == KW_IFJ) // inbuilt
    {
        fprintf(stderr, "INBUILD\n");
        (*nextToken) = getToken(file);
        advance(&DOT_INBUILD, nextToken, file);
        const char *prefix = "Ifj.";
        char *varName = (*nextToken)->id;
        advance(&ID_INBUILD, nextToken, file);

        size_t len = strlen(prefix) + strlen(varName) + 1;
        char *functionName = malloc(len);
        if (!functionName)
        {
            program_error(file, 0, 0, *nextToken);
        }

        strcpy(functionName, prefix);
        strcat(functionName, varName);

        // call initialize
        ASTptr inbuildCallNode = (ASTptr)malloc(sizeof(ASTnode));
        inbuildCallNode->type = AST_FUNC_CALL;
        inbuildCallNode->call.funcName = my_strdup(functionName);
        inbuildCallNode->call.argCap = 0;
        inbuildCallNode->call.argCount = 0;
        inbuildCallNode->call.callInfo = NULL;
        advance(&PARAN_INBUILD, nextToken, file);

        ArgArr argArr;
        argArrInit(&argArr);
        ARG(nextToken, file, &argArr);

        inbuildCallNode->call.argCount = argArr.arrCnt;
        inbuildCallNode->call.argCap = argArr.arrCnt;
        inbuildCallNode->call.args = argArr.items;

        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        return inbuildCallNode;
    }
    else if ((*nextToken)->type == NUMERICAL || (*nextToken)->type == STRING || (*nextToken)->type == KW_NULL)
    {
        fprintf(stderr, "numbers maison, what does they mean\n");
        ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
        advance(&END_TARGET, nextToken, file);
        return expresNode;
    }
    else if (peek(&PARAN_INBUILD, *nextToken, file))
    {
        fprintf(stderr, "parantesis maison, what does they mean\n");
        ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
        advance(&END_TARGET, nextToken, file);
        return expresNode;
    }
    else if ((*nextToken)->type == IDENTIFIER || (*nextToken)->type == ID_GLOBAL_VAR) // is it FUNC_CALL or epression?
    {
        fprintf(stderr, "rozhodnutie je to expression / call\n");
        char *varName = (*nextToken)->id;
        TokenPtr la = peekToken(file);
        if (la->type == SPECIAL && strcmp(la->id, "(") == 0) //
        {
            fprintf(stderr, "call\n");
            ASTptr callNode = (ASTptr)malloc(sizeof(ASTnode));
            callNode->type = AST_FUNC_CALL;
            callNode->call.funcName = my_strdup(varName);
            callNode->call.argCap = 0;
            callNode->call.argCount = 0;
            callNode->call.args = NULL;
            callNode->call.callInfo = NULL;

            ArgArr argArr;
            argArrInit(&argArr);

            *nextToken = getToken(file);
            FUNC_TYPE(nextToken, file, &argArr);

            callNode->call.argCap = argArr.arrCap;
            callNode->call.argCount = argArr.arrCnt;
            callNode->call.args = argArr.items;

            fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                    (*nextToken)->type,
                    (*nextToken)->id ? (*nextToken)->id : "NULL",
                    (*nextToken)->data ? (*nextToken)->data : "NULL");

            if ((*nextToken)->type != NEWLINE)
            {
                fprintf(stderr, "nemam newline\n");
                program_error(file, 2, 4, *nextToken);
            }
            (*nextToken) = getToken(file);
            return callNode;
        }
        else
        {
            fprintf(stderr, "expression\n");
            ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
            advance(&END_TARGET, nextToken, file);
            return expresNode;
        }
    }
    else
    {
        fprintf(stderr, "error in RSA\n");
        program_error(file, 2, 4, *nextToken);
    }
    return NULL;
}

/**
 * @brief Parses an optional function call argument list after an identifier.
 *
 * Implements the rule:
 *   FUNC_TYPE ::= ''
 *               | ( ARG )
 *
 * When the next symbol is '(', the function:
 *   - consumes '(',
 *   - parses arguments with ARG(),
 *   - and consumes the closing ')'.
 * Otherwise, if the next symbol is NEWLINE, the rule is epsilon (no args).
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 * @param argArr    Pointer to the argument array structure to be filled.
 */
void FUNC_TYPE(TokenPtr *nextToken, FILE *file, ArgArr *argArr)
{
    fprintf(stderr, "som v FUNC_TYPE\n");
    static const target FUNC_TYPE_FIRST = {SPECIAL, NULL, "("};
    static const target FUNC_TYPE_NEXT = {SPECIAL, NULL, ")"};
    if (peek(&FUNC_TYPE_FIRST, *nextToken, file)) // function
    {
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");

        advance(&FUNC_TYPE_FIRST, nextToken, file);

        fprintf(stderr, "idem do ARG\n");

        ARG(nextToken, file, argArr);

        fprintf(stderr, "vraciam sa do FUNC_TYPE\n");

        advance(&FUNC_TYPE_NEXT, nextToken, file);

        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        fprintf(stderr, "vraciam sa z FUNC_TYPE\n");

        return;
    }
    else if ((*nextToken)->type == NEWLINE) // epsilon -> getter
    {
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return;
}

/**
 * @brief Parses a (possibly empty) argument list for a function call.
 *
 * Implements the rules:
 *   ARG      ::= ''
 *              | ARG_NAME NEXT_ARG
 *   NEXT_ARG ::= ''
 *              | , ARG_NAME NEXT_ARG
 *
 * Each argument expression is converted into an AST node via fillNode()
 * and stored into @p argArr.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 * @param argArr    Pointer to the argument array structure being filled.
 */
void ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr)
{
    static const target ARG_FOLLOW = {SPECIAL, NULL, ")"};
    fprintf(stderr, "Som uz v ARG\n");

    if (ARG_NAME(nextToken, file))
    {
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        // create node
        ASTptr node = (ASTptr)malloc(sizeof(ASTnode));

        fillNode(&node, file, nextToken, argArr);

        *nextToken = getToken(file);
        NEXT_ARG(nextToken, file, argArr);
        return;
    }
    else if (peek(&ARG_FOLLOW, *nextToken, file)) // epsilon
    {
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return;
}

/**
 * @brief Parses additional arguments after the first one.
 *
 * Implements the rule:
 *   NEXT_ARG ::= , ARG_NAME NEXT_ARG | ''
 *
 * On ',', the function consumes the comma, parses the next ARG_NAME,
 * builds the corresponding AST node via fillNode(), and recurses.
 * If ')' is seen, the rule is epsilon.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 * @param argArr    Pointer to the argument array structure being filled.
 */
void NEXT_ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr)
{
    static const target NEXT_ARG_FIRST = {SPECIAL, NULL, ","};
    static const target NEXT_ARG_FOLLOW = {SPECIAL, NULL, ")"};
    if (peek(&NEXT_ARG_FIRST, *nextToken, file))
    {
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        *nextToken = getToken(file);
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        ARG_NAME(nextToken, file);

        ASTptr node = (ASTptr)malloc(sizeof(ASTnode));

        fillNode(&node, file, nextToken, argArr);

        *nextToken = getToken(file);
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        NEXT_ARG(nextToken, file, argArr);
        return;
    }
    else if (peek(&NEXT_ARG_FOLLOW, *nextToken, file)) // epsilon
    {
        return;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return;
}

/**
 * @brief Checks whether the current token is a valid argument value.
 *
 * Implements the rule:
 *   ARG_NAME ::= int
 *              | string
 *              | float
 *              | id
 *              | global_id
 *              | null
 *
 * In practice, this means tokens of type NUMERICAL, IDENTIFIER,
 * ID_GLOBAL_VAR, STRING or KW_NULL.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return 1 if the token matches any allowed argument kind, 0 otherwise.
 */
int ARG_NAME(TokenPtr *nextToken, FILE *file)
{
    fprintf(stderr, "som v arg name\n");
    static const target ARG_NAME_FIRST[] = {
        {NUMERICAL, NULL, NULL},
        {IDENTIFIER, NULL, NULL},
        {ID_GLOBAL_VAR, NULL, NULL},
        {STRING, NULL, NULL},
        {KW_NULL, "null", NULL}};

    size_t ARG_NAME_FIRST_LEN = sizeof(ARG_NAME_FIRST) / sizeof(ARG_NAME_FIRST[0]);

    if (nameHelperFunc(nextToken, ARG_NAME_FIRST, ARG_NAME_FIRST_LEN, file))
    {
        fprintf(stderr, "meno sedi\n");
        return 1;
    }
    fprintf(stderr, "meno nesedi gadzo\n");
    return 0;
}

/**
 * @brief Checks whether the current token is a valid variable name.
 *
 * Implements the rule:
 *   VAR_NAME ::= id
 *              | global_id
 *
 * @param nextToken Pointer to the current token pointer.
 * @param file      Source file handle used for error reporting.
 *
 * @return 1 if the token is IDENTIFIER or ID_GLOBAL_VAR, 0 otherwise.
 */
int VAR_NAME(TokenPtr *nextToken, FILE *file) //, FILE *file
{
    static const target VAR_NAME_SEQ[] = {
        {IDENTIFIER, NULL, NULL},
        {ID_GLOBAL_VAR, NULL, NULL}};

    size_t VAR_NAME_SEQ_LEN = sizeof(VAR_NAME_SEQ) / sizeof(VAR_NAME_SEQ[0]);

    if (nameHelperFunc(nextToken, VAR_NAME_SEQ, VAR_NAME_SEQ_LEN, file))
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Checks whether the current token matches any target specification.
 *
 * Iterates over a target array and calls peek() on each entry. If any of the
 * targets matches the current token, the function returns 1. The token is not
 * consumed.
 *
 * @param nextToken Pointer to the current token pointer.
 * @param target    Array of target descriptors to check against.
 * @param target_len Number of elements in the @p target array.
 * @param file      Source file handle used for error reporting.
 *
 * @return 1 if the token matches at least one target descriptor, 0 otherwise.
 */
int nameHelperFunc(TokenPtr *nextToken, const target *target, size_t target_len, FILE *file)
{
    size_t i = 0;
    while (i < target_len)
    {
        if (peek(&target[i], *nextToken, file))
        {
            return 1;
        }
        i++;
    }
    return 0;
}

/**
 * @brief Compares an expected token descriptor with the actual token.
 *
 * Checks whether the token type matches target->type, and optionally compares
 * either token->data or token->id with the corresponding target field
 * (if non-NULL). If a data or id comparison is required but the token
 * field is NULL, program_error() is called.
 *
 * @param target Expected token descriptor (type, id and/or data).
 * @param token  Actual token obtained from the lexer.
 * @param file   Source file handle used for error reporting.
 *
 * @return 1 if the token matches the descriptor, 0 otherwise.
 */
int peek(const target *target, TokenPtr token, FILE *file)
{
    if (target->type != token->type)
    {
        return 0;
    }
    else
    {
        if (target->data != NULL)
        {
            if (token->data == NULL)
            {
                program_error(file, 0, 0, token);
            }

            if (strcmp(target->data, token->data) != 0)
            {
                return 0;
            }
        }
        else if (target->id != NULL)
        {
            if (token->id == NULL)
            {
                program_error(file, 0, 0, token);
            }

            if (strcmp(target->id, token->id) != 0)
            {
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

/**
 * @brief Sequentially verifies and consumes a fixed token sequence.
 *
 * For each element in the @p TARGE_SEQ array, calls advance(), which both
 * checks the current token via peek() and consumes it if it matches. Used
 * to validate fixed patterns such as "class Program {" or "} else { EOL".
 *
 * @param TARGE_SEQ    Array of token descriptors that must appear in order.
 * @param file         Source file handle used for error reporting.
 * @param nextToken    Pointer to the current token pointer.
 * @param TARGE_SEQ_LEN Number of elements in the @p TARGE_SEQ array.
 */
void for_function(const target *TARGE_SEQ, FILE *file, TokenPtr *nextToken, size_t TARGE_SEQ_LEN)
{
    for (size_t i = 0; i < TARGE_SEQ_LEN; i++)
    {
        advance(&TARGE_SEQ[i], nextToken, file);
    }
}

/**
 * @brief Verifies that the current token matches the expected target and advances.
 *
 * First checks the token using peek(). If it does not match, a syntax error is
 * reported via program_error(). If it matches, the token is consumed by calling
 * getToken(), and @p token is updated accordingly.
 *
 * @param target Expected token descriptor.
 * @param token  Pointer to the current token pointer to be advanced.
 * @param file   Source file handle used for error reporting.
 */
void advance(const target *target, TokenPtr *token, FILE *file)
{
    if (!peek(target, *token, file))
    {
        program_error(file, 2, 4, *token);
    }
    *token = getToken(file);
}

/**
 * @brief Returns the next token, taking into account a pending lookahead.
 *
 * If a token has been previously read by peekToken() and stored in the global
 * lookahead buffer, that token is returned and the pending flag is cleared.
 * Otherwise, a new token is read from the lexer().
 *
 * @param file Source file handle used by the lexer.
 *
 * @return Next token from the input stream.
 */
TokenPtr getToken(FILE *file)
{
    if (pending == true)
    {
        pending = false;
        return lookahead;
    }
    return lexer(file);
}

/**
 * @brief Returns the next token without consuming it permanently.
 *
 * If there is no pending lookahead token, calls lexer() to obtain one,
 * stores it in the global lookahead variable, and sets the pending flag.
 * Subsequent calls to getToken() will return this token first.
 *
 * @param file Source file handle used by the lexer.
 *
 * @return Lookahead token.
 */
TokenPtr peekToken(FILE *file)
{
    if (pending == false)
    {
        lookahead = lexer(file);
        pending = true;
    }
    return lookahead;
}

/**
 * @brief Skips consecutive NEWLINE tokens in the input.
 *
 * Repeatedly calls getToken() while the current token is of type NEWLINE.
 * Used to normalize places where extra blank lines are allowed by the syntax.
 *
 * @param file      Source file handle used by the lexer.
 * @param nextToken Pointer to the current token pointer; updated in-place.
 */
void skip_newline(FILE *file, TokenPtr *nextToken)
{
    while ((*nextToken)->type == NEWLINE)
    {
        (*nextToken) = getToken(file);
    }
    return;
}

/**
 * @brief Creates and appends an AST node representing an argument expression.
 *
 * Based on the type of @p *nextToken, this function fills an already allocated
 * AST node (@p *node) as either:
 *   - a numeric literal (NUMERICAL),
 *   - a string literal (STRING),
 *   - a local or global identifier (IDENTIFIER, ID_GLOBAL_VAR),
 *   - or a null literal (KW_NULL).
 *
 * After initializing the node, it ensures that @p argArr has enough capacity
 * (reallocating its internal array if needed) and appends the node to the
 * argument array.
 *
 * @param node     Pointer to an already allocated AST node pointer to fill.
 * @param file     Source file handle used for error reporting.
 * @param nextToken Pointer to the current token pointer (describing the value).
 * @param argArr   Pointer to the argument array structure where the node is stored.
 */
void fillNode(ASTptr *node, FILE *file, TokenPtr *nextToken, ArgArr *argArr)
{
    fprintf(stderr, "Som uz v fillnode\n");
    if ((*nextToken)->type == NUMERICAL)
    {
        fprintf(stderr, "Som v literali\n");
        fprintf(stderr, "token: type=%d, id=%s, data=%s\n",
                (*nextToken)->type,
                (*nextToken)->id ? (*nextToken)->id : "NULL",
                (*nextToken)->data ? (*nextToken)->data : "NULL");
        (*node)->type = AST_LITERAL;
        (*node)->literal.liType = LIT_NUMBER;
        (*node)->literal.num = strtod((*nextToken)->data, NULL);
        fprintf(stderr, "literal.num = %f\n", (*node)->literal.num);
    }
    else if ((*nextToken)->type == STRING)
    {
        (*node)->type = AST_LITERAL;
        (*node)->literal.liType = LIT_STRING;
        (*node)->literal.num = 0;
        (*node)->literal.str = my_strdup((*nextToken)->data);
    }
    else if ((*nextToken)->type == ID_GLOBAL_VAR)
    {
        (*node)->type = AST_IDENTIFIER;
        (*node)->identifier.idType = ID_GLOBAL;
        (*node)->identifier.name = my_strdup((*nextToken)->id);
    }
    else if ((*nextToken)->type == IDENTIFIER)
    {
        (*node)->type = AST_IDENTIFIER;
        (*node)->identifier.idType = ID_LOCAL;
        (*node)->identifier.name = my_strdup((*nextToken)->id);
    }
    else if ((*nextToken)->type == KW_NULL)
    {
        (*node)->type = AST_LITERAL;
        (*node)->literal.liType = LIT_NULL;
    }

    if (argArr->arrCnt == argArr->arrCap)
    {
        int newCap;
        if (argArr->arrCap == 0)
        {
            newCap = 4;
        }
        else
        {
            newCap = argArr->arrCap * 2;
        }

        ASTptr *newArr = realloc(argArr->items, newCap * sizeof(ASTptr));
        if (!newArr)
        {
            program_error(file, 99, 0, *nextToken);
        }
        argArr->items = newArr;
        argArr->arrCap = newCap;
    }
    argArr->items[argArr->arrCnt] = *node;
    argArr->arrCnt++;
    fprintf(stderr, "odchadzam z fillnode\n");
    return;
}