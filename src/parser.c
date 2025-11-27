//////////////////////////////////////////////
// filename: parser.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
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

/*NOTES:
   - once in time should call semantic analyze to make AST from simulated derivation tree -> quit confused how to do that
   ├─ shouldn't it create an AST during analysis, i.e., when it recognizes the structure, it creates a suitable AST node? -> A: yes, it will be call at the end of function
   └─ maybe change funciton types from int to ASTptr so we can assemble the tree -> A: yes but first i want to finish LL rules
   - problem with arg_name using peek function check if there is valid token type if yes token will be processed (token will be used to created AST node ), must thing of way how to processed all arg at once or not
   - PROBLEM WITH '(' in assign -> should call expression_parse
*/

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

ASTptr
parser(FILE *file)
{
    TokenPtr token = getToken(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    while (token->type == NEWLINE)
    {
        token = getToken(file);
    }
    ASTptr root = PROGRAM(&token, file);
    printf("Syntakticka analyza prebehla uspesne!!!\n");
    return root;
}

ASTptr PROGRAM(TokenPtr *nextToken, FILE *file)
{
    printf("som v programe\n");
    PROLOG(nextToken, file);

    ASTptr program = (ASTptr)malloc(sizeof(ASTnode));

    program = CLASS(nextToken, file);

    return program;
}

void PROLOG(TokenPtr *nextToken, FILE *file)
{
    static const target PROLOG_TARGET[] = {
        {KW_IMPORT, NULL, "import"},
        {STRING, "ifj25", NULL},
        {KW_FOR, NULL, "for"},
        {KW_IFJ, NULL, "Ifj"},
        {NEWLINE, NULL, NULL}};

    static const size_t PROLOG_TARGET_LEN = sizeof(PROLOG_TARGET) / sizeof(PROLOG_TARGET[0]);

    for_function(PROLOG_TARGET, file, nextToken, PROLOG_TARGET_LEN);

    return;
}

ASTptr CLASS(TokenPtr *nextToken, FILE *file) // change return type to ASTnode
{
    // TODO make while and array filled with "class Program { EOL". While would iterated until array is passted then function FUNCTIONS is called and after that check for }

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

ASTptr FUNCTIONS(TokenPtr *nextToken, FILE *file, ASTptr programNode) // change return type to ASTnode
{
    static const target FUNCTIONS_FOLLOW = {SPECIAL, NULL, "}"};

    if ((*nextToken)->type == KW_STATIC)
    {
        static const target FUNCTIONS_FIRST = {KW_STATIC, NULL, "static"};
        advance(&FUNCTIONS_FIRST, nextToken, file);

        TokenPtr funcName = *nextToken;

        if (FUNC_NAME(nextToken, file)) // dont forget to iterate nextToken inside this function!!!
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

        FUNC_GET_SET_DEF(nextToken, file, function); // dont forget to iterate nextToken inside this function!!!

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

int FUNC_NAME(TokenPtr *nextToken, FILE *file)
{
    if ((*nextToken)->type == IDENTIFIER)
    {
        // advance(&FUNC_NAME_TARGET, nextToken, file); // is nextToken pointer?
        *nextToken = getToken(file);
        return 0;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return 1;
}

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

    if (peek(&FUNC_DEF, *nextToken, file)) // definition of function -> should call semantic analyzer to check if function id is already used -> beware of shadowing!!!
    {
        *nextToken = getToken(file);

        // Loading par of function into the AST
        parArr pA;
        parArrInit(&pA);
        PAR(nextToken, file, &pA); // dont forget to iterate nextToken inside this function!!!
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
        FUNC_BODY(nextToken, file, blockNode); // dont forget to iterate nextToken inside this function!!!
        printf("vratil som sa z body\n");
        functionNode->func.body = blockNode;

        printf("idem na koniec toto je v tokene\n");
        printf("token: type=%d, id=%s, data=%s\n",
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
        FUNC_BODY(nextToken, file, blockNode); // dont forget to iterate nextToken inside this function!!!
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
        FUNC_BODY(nextToken, file, blockNode); // dont forget to iterate nextToken inside this function!!!
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

ASTptr PAR(TokenPtr *nextToken, FILE *file, parArr *pA)
{
    static const target PAR_FIRST = {IDENTIFIER, NULL, NULL};
    static const target PAR_FOLLOW = {SPECIAL, NULL, ")"};
    if ((*nextToken)->type == IDENTIFIER)
    {
        char *name = my_strdup((*nextToken)->id);
        parArrAdd(pA, name, file, *nextToken);

        advance(&PAR_FIRST, nextToken, file);

        return NEXT_PAR(nextToken, file, pA); // dont forget to iterate nextToken inside this function!!!
    }
    else if (peek(&PAR_FOLLOW, *nextToken, file))
    {
        return NULL;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return NULL;
}

ASTptr NEXT_PAR(TokenPtr *nextToken, FILE *file, parArr *pA)
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

        return NEXT_PAR(nextToken, file, pA);
    }
    else if (peek(&NEXT_PAR_FOLLOW, *nextToken, file)) // epsilon
    {
        return NULL;
    }
    else
    {
        program_error(file, 2, 4, *nextToken);
    }
    return NULL;
}

ASTptr FUNC_BODY(TokenPtr *nextToken, FILE *file, ASTptr blockNode)
{

    static const target END_IF_EXP = {SPECIAL, NULL, ")"};
    static const target RETURN_FIRST = {KW_RETURN, NULL, NULL};
    static const target FUNC_BODY_END = {NEWLINE, NULL, NULL};
    static const target FUNC_BODY_FOLLOW = {SPECIAL, NULL, "}"};
    static const target VAR_ASS_CALL_GET = {SPECIAL, NULL, "="};
    static const target END_RETURN_EXP = {NEWLINE, NULL, NULL};

    /*static const target FUNC_INTRO_SEQ[] =
        {
            {SPECIAL, "(", NULL},
            {NEWLINE, NULL, NULL}};*/

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

    static const target WHILE_START_SEQ[] =
        {
            {KW_WHILE, NULL, NULL},
            {SPECIAL, NULL, "("}};

    size_t IF_STATMENT_ELSE_BRANCH_SEQ_LEN = sizeof(IF_STATMENT_ELSE_BRANCH_SEQ) / sizeof(IF_STATMENT_ELSE_BRANCH_SEQ[0]);
    size_t IF_STATMENT_MIDDLE_SEQ_LEN = sizeof(IF_STATMENT_MIDDLE_SEQ) / sizeof(IF_STATMENT_MIDDLE_SEQ[0]);
    size_t IF_STATMENT_START_SEQ_LEN = sizeof(IF_STATMENT_START_SEQ) / sizeof(IF_STATMENT_START_SEQ[0]);
    // size_t VAR_ASS_CALL_GET_SEQ_LEN = sizeof(VAR_ASS_CALL_GET_SEQ) / sizeof(VAR_ASS_CALL_GET_SEQ[0]);
    // size_t FUNC_BODY_DECL_SEQ_LEN = sizeof(FUNC_BODY_DECL_SEQ) / sizeof(FUNC_BODY_DECL_SEQ[0]);
    size_t WHILE_START_SEQ_LEN = sizeof(WHILE_START_SEQ) / sizeof(WHILE_START_SEQ[0]);
    // size_t FUNC_INTRO_SEQ_LEN = sizeof(FUNC_INTRO_SEQ) / sizeof(FUNC_INTRO_SEQ[0]);
    size_t END_SEQ_LEN = sizeof(END_SEQ) / sizeof(END_SEQ[0]);

    printf("idem do body\n");
    if ((*nextToken)->type == KW_VAR) // declare
    {
        printf("deklaracia\n");
        //  for_function(FUNC_BODY_DECL_SEQ, file, nextToken, FUNC_BODY_DECL_SEQ_LEN);
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
    else if (VAR_NAME(nextToken, file)) // assign
    {
        printf("priradenie\n");
        AssignTargetType asType;
        if ((*nextToken)->type == ID_GLOBAL_VAR)
        {
            asType = TARGET_GLOBAL;
        }
        else
        {
            asType = TARGET_LOCAL;
        }
        char *varName = (*nextToken)->id;
        ASTptr assignNode = (ASTptr)malloc(sizeof(ASTnode));
        assignNode->type = AST_ASSIGN_STMT;
        assignNode->assign_stmt.targetName = my_strdup(varName);
        assignNode->assign_stmt.asType = asType;

        *nextToken = getToken(file);
        advance(&VAR_ASS_CALL_GET, nextToken, file);
        ASTptr exprNode = RSA(nextToken, file);

        assignNode->assign_stmt.expr = exprNode;

        varNameAdd(blockNode, assignNode, file, *nextToken);
        printf("vraciam sa z body\n");
        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_IF) // if statement
    {
        printf("if\n");
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
        printf("while\n");
        ASTptr whileNode = (ASTptr)malloc(sizeof(ASTnode));
        whileNode->type = AST_WHILE_STMT;

        for_function(WHILE_START_SEQ, file, nextToken, WHILE_START_SEQ_LEN);

        ASTptr condition = parse_expression(nextToken, file, &END_IF_EXP);
        whileNode->while_stmt.cond = condition;

        for_function(IF_STATMENT_MIDDLE_SEQ, file, nextToken, IF_STATMENT_MIDDLE_SEQ_LEN);

        ASTptr whileBody = (ASTptr)malloc(sizeof(ASTnode));
        blockNodeInit(whileBody);
        FUNC_BODY(nextToken, file, whileBody);

        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);

        varNameAdd(blockNode, whileNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if ((*nextToken)->type == KW_RETURN) // return
    {
        printf("return\n");
        ASTptr returnNode = (ASTptr)malloc(sizeof(ASTnode));
        returnNode->type = AST_RETURN_STMT;

        advance(&RETURN_FIRST, nextToken, file);

        ASTptr condition = parse_expression(nextToken, file, &END_RETURN_EXP);
        returnNode->return_stmt.expr = condition;

        advance(&FUNC_BODY_END, nextToken, file);

        varNameAdd(blockNode, returnNode, file, *nextToken);

        return FUNC_BODY(nextToken, file, blockNode);
    }
    else if (peek(&FUNC_BODY_FOLLOW, *nextToken, file)) // epsilon
    {
        printf("som v epsilone\n");
        return NULL;
    }
    else
    {
        printf("skoncil som v chybe pre FUNC_BODY\n");
        program_error(file, 2, 4, *nextToken);
    }
    printf("dostal som sa sem?\n");
    return NULL;
}

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

    // working code -> should refactor so it isnt so blouted
    printf("som v RSA\n");
    if ((*nextToken)->type == KW_IFJ) // inbuilt
    {
        printf("INBUILD\n");
        (*nextToken) = getToken(file);
        advance(&DOT_INBUILD, nextToken, file);
        char *varName = (*nextToken)->id;
        advance(&ID_INBUILD, nextToken, file);

        // call initialize -> maybe make function;
        ASTptr inbuildCallNode = (ASTptr)malloc(sizeof(ASTnode));
        inbuildCallNode->type = AST_FUNC_CALL;
        inbuildCallNode->call.funcType = FUNC_INBUILD;
        inbuildCallNode->call.funcName = my_strdup(varName);
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

        /* if (inbuildCallNode->call.argCount > 0)
        {
            inbuildCallNode->call.args = malloc(argArr.arrCnt * sizeof(ASTptr));
        }
        else
        {
            inbuildCallNode->call.args = NULL;
        }

        for (int i = 0; i < inbuildCallNode->call.argCount; i++)
        {
            ASTptr item = (ASTptr)malloc(sizeof(ASTnode));
            item->type = AST_LITERAL;
            item->literal.liType = argArr.items[i].liType;
            if (item->literal.liType == LIT_NUMBER)
            {
                item->literal.num = argArr.items[i].num;
            }
            else if (item->literal.liType == LIT_STRING)
            {
                item->literal.str = argArr.items[i].str;
            }
            else if (item->literal.liType == LIT_LOCAL_ID)
            {
                item->literal.str = argArr.items[i].str;
                item->literal.num = 0;
            }
            else if (item->literal.liType == LIT_GLOBAL_ID)
            {
                item->literal.str = argArr.items[i].str;
                item->literal.num = 0;
            }
            inbuildCallNode->call.args[i] = item;
        } */

        for_function(END_SEQ, file, nextToken, END_SEQ_LEN);
        return inbuildCallNode;
    }
    else if ((*nextToken)->type == NUMERICAL || (*nextToken)->type == STRING || (*nextToken)->type == KW_NULL) // if num = expression parsing
    {
        printf("numbers maison, what does they mean\n");
        ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
        advance(&END_TARGET, nextToken, file);
        return expresNode;
    }
    else if (peek(&PARAN_INBUILD, *nextToken, file))
    {
        printf("parantesis maison, what does they mean\n");
        ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
        advance(&END_TARGET, nextToken, file);
        return expresNode;
    }
    else if ((*nextToken)->type == IDENTIFIER || (*nextToken)->type == ID_GLOBAL_VAR) // is it FUNC CALL or epression?
    {
        printf("rozhodnutie je to expression / call\n");
        char *varName = (*nextToken)->id; // this will needs to be cleared if not function call??
        TokenPtr la = peekToken(file);
        if (la->type == SPECIAL && strcmp(la->id, "(") == 0) //
        {
            printf("call\n");
            ASTptr callNode = (ASTptr)malloc(sizeof(ASTnode));
            callNode->type = AST_FUNC_CALL;
            callNode->call.funcType = FUNC_USER;
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

            /* if (callNode->call.argCount > 0)
            {
                callNode->call.args = malloc(argArr.arrCnt * sizeof(ASTptr));
            }
            else
            {
                callNode->call.args = NULL;
            }

            for (int i = 0; i < callNode->call.argCount; i++)
            {
                ASTptr item = (ASTptr)malloc(sizeof(ASTnode));
                item->type = AST_LITERAL;
                item->literal.liType = argArr.items[i].liType;
                if (item->literal.liType == LIT_NUMBER)
                {
                    item->literal.num = argArr.items[i].num;
                    item->literal.str = "";
                }
                else if (item->literal.liType == LIT_STRING)
                {
                    item->literal.str = argArr.items[i].str;
                    item->literal.num = 0;
                }
                else if (item->literal.liType == LIT_LOCAL_ID)
                {
                    item->literal.str = argArr.items[i].str;
                    item->literal.num = 0;
                }
                else if (item->literal.liType == LIT_GLOBAL_ID)
                {
                    item->literal.str = argArr.items[i].str;
                    item->literal.num = 0;
                }
                callNode->call.args[i] = item;
            printf("som vo vnutri i guess\n");
        }*/
            printf("token: type=%d, id=%s, data=%s\n",
                   (*nextToken)->type,
                   (*nextToken)->id ? (*nextToken)->id : "NULL",
                   (*nextToken)->data ? (*nextToken)->data : "NULL");

            if ((*nextToken)->type != NEWLINE)
            {
                printf("nemam newline\n");
                program_error(file, 2, 4, *nextToken);
            }
            (*nextToken) = getToken(file);
            return callNode;
        }
        else
        {
            printf("expression\n");
            ASTptr expresNode = parse_expression(nextToken, file, &END_TARGET);
            advance(&END_TARGET, nextToken, file);
            return expresNode;
        }
    } // there can be a expression -> give control to PSA
    else
    {
        printf("jebol som kokot\n");
        program_error(file, 2, 4, *nextToken);
    }
    return NULL;
}

void FUNC_TYPE(TokenPtr *nextToken, FILE *file, ArgArr *argArr)
{
    printf("som v FUNC_TYPE\n");
    static const target FUNC_TYPE_FIRST = {SPECIAL, NULL, "("};
    static const target FUNC_TYPE_NEXT = {SPECIAL, NULL, ")"};
    if (peek(&FUNC_TYPE_FIRST, *nextToken, file)) // function
    {
        printf("token: type=%d, id=%s, data=%s\n",
               (*nextToken)->type,
               (*nextToken)->id ? (*nextToken)->id : "NULL",
               (*nextToken)->data ? (*nextToken)->data : "NULL");

        advance(&FUNC_TYPE_FIRST, nextToken, file);
        printf("idem do ARG\n");
        ARG(nextToken, file, argArr);
        printf("vraciam sa do FUNC_TYPE\n");
        advance(&FUNC_TYPE_NEXT, nextToken, file);
        printf("token: type=%d, id=%s, data=%s\n",
               (*nextToken)->type,
               (*nextToken)->id ? (*nextToken)->id : "NULL",
               (*nextToken)->data ? (*nextToken)->data : "NULL");
        printf("vraciam sa z FUNC_TYPE\n");
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

void ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr)
{
    static const target ARG_FOLLOW = {SPECIAL, NULL, ")"};
    printf("Som uz v ARG\n");

    if (ARG_NAME(nextToken, file))
    {
        printf("token: type=%d, id=%s, data=%s\n",
               (*nextToken)->type,
               (*nextToken)->id ? (*nextToken)->id : "NULL",
               (*nextToken)->data ? (*nextToken)->data : "NULL");
        // create node
        ASTptr node = (ASTptr)malloc(sizeof(ASTnode));
        if ((*nextToken)->type == NUMERICAL)
        {
            node->type = AST_LITERAL;
            node->literal.liType = LIT_NUMBER;
            node->literal.num = strtod((*nextToken)->data, NULL);
            node->literal.str = NULL;
        }
        else if ((*nextToken)->type == STRING)
        {
            node->type = AST_LITERAL;
            node->literal.liType = LIT_STRING;
            node->literal.num = 0;
            node->literal.str = (*nextToken)->data;
        }
        else if ((*nextToken)->type == ID_GLOBAL_VAR)
        {
            node->type = AST_IDENTIFIER;
            node->identifier.idType = ID_GLOBAL;
            node->identifier.name = (*nextToken)->id;
        }
        else if ((*nextToken)->type == IDENTIFIER)
        {
            node->type = AST_IDENTIFIER;
            node->identifier.idType = ID_LOCAL;
            node->identifier.name = (*nextToken)->id;
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
                program_error(file, 0, 0, *nextToken);
            }
            argArr->items = newArr;
            argArr->arrCap = newCap;
        }

        argArr->items[argArr->arrCnt] = node;
        argArr->arrCnt++;

        *nextToken = getToken(file);
        NEXT_ARG(nextToken, file, argArr);
        return; // nextToken was itareted in this function
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

void NEXT_ARG(TokenPtr *nextToken, FILE *file, ArgArr *argArr) // change return type to ASTnode
{
    static const target NEXT_ARG_FIRST = {SPECIAL, NULL, ","}; // look at it again please
    static const target NEXT_ARG_FOLLOW = {SPECIAL, NULL, ")"};
    if (peek(&NEXT_ARG_FIRST, *nextToken, file))
    {
        printf("token: type=%d, id=%s, data=%s\n",
               (*nextToken)->type,
               (*nextToken)->id ? (*nextToken)->id : "NULL",
               (*nextToken)->data ? (*nextToken)->data : "NULL");
        *nextToken = getToken(file);
        printf("token: type=%d, id=%s, data=%s\n",
               (*nextToken)->type,
               (*nextToken)->id ? (*nextToken)->id : "NULL",
               (*nextToken)->data ? (*nextToken)->data : "NULL");
        ARG_NAME(nextToken, file);

        /* literal lit;
        if ((*nextToken)->type == NUMERICAL)
        {
            lit.liType = LIT_NUMBER;
            lit.num = strtod((*nextToken)->data, NULL);
        }
        else if ((*nextToken)->type == STRING)
        {
            lit.liType = LIT_STRING;
            lit.str = (*nextToken)->data;
        }
        else if ((*nextToken)->type == ID_GLOBAL_VAR)
        {
            lit.liType = LIT_GLOBAL_ID;
            lit.str = (*nextToken)->id;
        }
        else if ((*nextToken)->type == IDENTIFIER)
        {
            lit.liType = LIT_LOCAL_ID;
            lit.str = (*nextToken)->id;
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

            literal *newArr = realloc(argArr->items, newCap * sizeof(literal));
            if (!newArr)
            {
                program_error(file, 0, 0, *nextToken);
            }
            argArr->items = newArr;
            argArr->arrCap = newCap;
        }

        argArr->items[argArr->arrCnt] = lit;
        argArr->arrCnt++; */

        ASTptr node = (ASTptr)malloc(sizeof(ASTnode));
        if ((*nextToken)->type == NUMERICAL)
        {
            node->type = AST_LITERAL;
            node->literal.liType = LIT_NUMBER;
            node->literal.num = strtod((*nextToken)->data, NULL);
            node->literal.str = NULL;
        }
        else if ((*nextToken)->type == STRING)
        {
            node->type = AST_LITERAL;
            node->literal.liType = LIT_STRING;
            node->literal.num = 0;
            node->literal.str = (*nextToken)->data;
        }
        else if ((*nextToken)->type == ID_GLOBAL_VAR)
        {
            node->type = AST_IDENTIFIER;
            node->identifier.idType = ID_GLOBAL;
            node->identifier.name = (*nextToken)->id;
        }
        else if ((*nextToken)->type == IDENTIFIER)
        {
            node->type = AST_IDENTIFIER;
            node->identifier.idType = ID_LOCAL;
            node->identifier.name = (*nextToken)->id;
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
                program_error(file, 0, 0, *nextToken);
            }
            argArr->items = newArr;
            argArr->arrCap = newCap;
        }

        argArr->items[argArr->arrCnt] = node;
        argArr->arrCnt++;

        *nextToken = getToken(file);
        printf("token: type=%d, id=%s, data=%s\n",
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

int ARG_NAME(TokenPtr *nextToken, FILE *file)
{
    printf("som v arg name\n");
    static const target ARG_NAME_FIRST[] = {
        {NUMERICAL, NULL, NULL},
        {IDENTIFIER, NULL, NULL},
        {ID_GLOBAL_VAR, NULL, NULL},
        {STRING, NULL, NULL}};

    size_t ARG_NAME_FIRST_LEN = sizeof(ARG_NAME_FIRST) / sizeof(ARG_NAME_FIRST[0]);

    if (nameHelperFunc(nextToken, ARG_NAME_FIRST, ARG_NAME_FIRST_LEN, file))
    {
        printf("meno sedi\n");
        return 1;
    }
    printf("meno nesedi gadzo\n");
    // program_error(file, 2, 4, *nextToken);
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
    // program_error(file, 2, 4, *nextToken);
    return 0;
}

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

// using strcmp compares target string with token. If token isn`t same as target string error is send to stderr output
/**
 * @brief checks if expected terminal and actual terminal are same
 * @param target expected terminal
 * @param token actual terminal
 *
 * @return wrong terminal will trigger stderr and program will end, correct terminal will return 1
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
                /*fprintf(stderr, "SEGFAULT, token pointer is empty and you are trying reach something you can`t idiot\n");
                exit(2);*/
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
                /*fprintf(stderr, "SEGFAULT, token pointer is empty and you are trying reach something you can`t idiot\n");
                exit(2);*/
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
    if (!peek(target, *token, file))
    {
        program_error(file, 2, 4, *token);
    }
    *token = getToken(file);
}

TokenPtr getToken(FILE *file)
{
    if (pending == true)
    {
        pending = false;
        return lookahead;
    }
    return lexer(file);
}

TokenPtr peekToken(FILE *file)
{
    if (pending == false)
    {
        lookahead = lexer(file);
        pending = true;
    }
    return lookahead;
}
