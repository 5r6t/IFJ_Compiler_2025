#include <stdio.h>
#include "ast.h"

// THIS FILE WAS GENERATED WITH AI -> USED FOR DEBUG. DO NOT SEND IT TO ZBYNEK!!!!!

static const char *astTypeToString(ASTnodeType type)
{
    switch (type)
    {
    case AST_PROGRAM:
        return "PROGRAM";
    case AST_FUNC_DEF:
        return "FUNC_DEF";
    case AST_FUNC_CALL:
        return "FUNC_CALL";
    case AST_BLOCK:
        return "BLOCK";
    case AST_IF_STMT:
        return "IF_STMT";
    case AST_RETURN_STMT:
        return "RETURN_STMT";
    case AST_VAR_DECL:
        return "VAR_DECL";
    case AST_ASSIGN_STMT:
        return "ASSIGN_STMT";
    case AST_WHILE_STMT:
        return "WHILE_STMT";
    case AST_IDENTIFIER:
        return "IDENTIFIER";
    case AST_LITERAL:
        return "LITERAL";
    case AST_BINOP:
        return "BINOP";
    default:
        return "UNKNOWN";
    }
}

static const char *assignTargetToString(AssignTargetType t)
{
    switch (t)
    {
    case TARGET_LOCAL:
        return "LOCAL";
    case TARGET_GLOBAL:
        return "GLOBAL";
    case TARGET_SETTER:
        return "SETTER";
    default:
        return "UNKNOWN";
    }
}

static const char *idTypeToString(IdType t)
{
    switch (t)
    {
    case ID_LOCAL:
        return "LOCAL";
    case ID_GLOBAL:
        return "GLOBAL";
    case ID_GETTER:
        return "GETTER";
    default:
        return "UNKNOWN";
    }
}

static const char *funcTypeToString(FuncType t)
{
    switch (t)
    {
    case FUNC_USER:
        return "USER";
    case FUNC_INBUILD:
        return "INBUILD";
    default:
        return "UNKNOWN";
    }
}

static const char *literalTypeToString(LiteralType t)
{
    switch (t)
    {
    case LIT_NULL:
        return "NULL";
    case LIT_LOCAL_ID:
        return "LOCAL_ID";
    case LIT_GLOBAL_ID:
        return "GLOBAL_ID";
    case LIT_NUMBER:
        return "NUMBER";
    case LIT_STRING:
        return "STRING";
    default:
        return "UNKNOWN";
    }
}

static const char *typeNameToString(TypeName t)
{
    switch (t)
    {
    case TYPE_STRING:
        return "string";
    case TYPE_NUMBER:
        return "number";
    case TYPE_NULL:
        return "null";
    default:
        return "unknown";
    }
}

static const char *binOpToString(BinOpType op)
{
    switch (op)
    {
    case BINOP_ADD:
        return "+";
    case BINOP_SUB:
        return "-";
    case BINOP_MUL:
        return "*";
    case BINOP_DIV:
        return "/";
    case BINOP_LT:
        return "<";
    case BINOP_GT:
        return ">";
    case BINOP_EQ:
        return "==";
    case BINOP_NEQ:
        return "!=";
    case BINOP_LTE:
        return "<=";
    case BINOP_GTE:
        return ">=";
    case BINOP_AND:
        return "&&";
    case BINOP_OR:
        return "||";
    case BINOP_IS:
        return "is";
    default:
        return "?";
    }
}

static void astIndent(int depth)
{
    for (int i = 0; i < depth; ++i)
    {
        fputs("  ", stderr); // 2 medzery na úroveň
    }
}

void astPrint(ASTptr node, int depth)
{
    if (node == NULL)
    {
        astIndent(depth);
        fprintf(stderr, "(null)\n");
        return;
    }

    astIndent(depth);
    fprintf(stderr, "%s", astTypeToString(node->type));

    /* základný riadok – doplnkové info */
    switch (node->type)
    {
    case AST_PROGRAM:
        fprintf(stderr, " (funcs=%d)", node->program.funcsCount);
        break;

    case AST_FUNC_DEF:
        fprintf(stderr, " name=%s params=%d",
                node->func.name ? node->func.name : "<null>",
                node->func.paramCount);
        if (node->func.isGetter)
            fprintf(stderr, " [getter]");
        if (node->func.isSetter)
            fprintf(stderr, " [setter]");
        break;

    case AST_FUNC_CALL:
        fprintf(stderr, " %s func=%s args=%d",
                funcTypeToString(node->call.funcType),
                node->call.funcName ? node->call.funcName : "<null>",
                node->call.argCount);
        break;

    case AST_BLOCK:
        fprintf(stderr, " (stmts=%d)", node->block.stmtCount);
        break;

    case AST_IF_STMT:
        fprintf(stderr, " (if)");
        break;

    case AST_WHILE_STMT:
        fprintf(stderr, " (while)");
        break;

    case AST_RETURN_STMT:
        fprintf(stderr, " (return)");
        break;

    case AST_VAR_DECL:
        fprintf(stderr, " name=%s",
                node->var_decl.varName ? node->var_decl.varName : "<null>");
        break;

    case AST_ASSIGN_STMT:
        fprintf(stderr, " target=%s [%s]",
                node->assign_stmt.targetName ? node->assign_stmt.targetName : "<null>",
                assignTargetToString(node->assign_stmt.asType));
        break;

    case AST_IDENTIFIER:
        fprintf(stderr, " name=%s [%s]",
                node->identifier.name ? node->identifier.name : "<null>",
                idTypeToString(node->identifier.idType));
        break;

    case AST_LITERAL:
        fprintf(stderr, " %s", literalTypeToString(node->literal.liType));
        if (node->literal.liType == LIT_NUMBER)
        {
            fprintf(stderr, " value=%g", node->literal.num);
        }
        else if (node->literal.liType == LIT_STRING)
        {
            fprintf(stderr, " value=\"%s\"",
                    node->literal.str ? node->literal.str : "");
        }
        break;

    case AST_BINOP:
        fprintf(stderr, " op=%s", binOpToString(node->binop.opType));
        if (node->binop.opType == BINOP_IS)
        {
            fprintf(stderr, " resultType=%s",
                    typeNameToString(node->binop.resultType));
        }
        break;

    default:
        break;
    }

    fputc('\n', stderr);

    /* rekurzívne deti podľa typu */
    switch (node->type)
    {
    case AST_PROGRAM:
        for (int i = 0; i < node->program.funcsCount; ++i)
        {
            astPrint(node->program.funcs[i], depth + 1);
        }
        break;

    case AST_FUNC_DEF:
        if (node->func.body)
        {
            astPrint(node->func.body, depth + 1);
        }
        break;

    case AST_FUNC_CALL:
        for (int i = 0; i < node->call.argCount; ++i)
        {
            astPrint(node->call.args[i], depth + 1);
        }
        break;

    case AST_BLOCK:
        for (int i = 0; i < node->block.stmtCount; ++i)
        {
            astPrint(node->block.stmt[i], depth + 1);
        }
        break;

    case AST_IF_STMT:
        /* cond */
        if (node->ifstmt.cond)
        {
            astIndent(depth + 1);
            fprintf(stderr, "[cond]\n");
            astPrint(node->ifstmt.cond, depth + 2);
        }
        /* then */
        if (node->ifstmt.then)
        {
            astIndent(depth + 1);
            fprintf(stderr, "[then]\n");
            astPrint(node->ifstmt.then, depth + 2);
        }
        /* else */
        if (node->ifstmt.elsestmt)
        {
            astIndent(depth + 1);
            fprintf(stderr, "[else]\n");
            astPrint(node->ifstmt.elsestmt, depth + 2);
        }
        break;

    case AST_WHILE_STMT:
        if (node->while_stmt.cond)
        {
            astIndent(depth + 1);
            fprintf(stderr, "[cond]\n");
            astPrint(node->while_stmt.cond, depth + 2);
        }
        if (node->while_stmt.body)
        {
            astIndent(depth + 1);
            fprintf(stderr, "[body]\n");
            astPrint(node->while_stmt.body, depth + 2);
        }
        break;

    case AST_RETURN_STMT:
        if (node->return_stmt.expr)
        {
            astPrint(node->return_stmt.expr, depth + 1);
        }
        break;

    case AST_ASSIGN_STMT:
        if (node->assign_stmt.expr)
        {
            astPrint(node->assign_stmt.expr, depth + 1);
        }
        break;

    case AST_BINOP:
        if (node->binop.left)
        {
            astPrint(node->binop.left, depth + 1);
        }
        if (node->binop.right)
        {
            astPrint(node->binop.right, depth + 1);
        }
        break;

    case AST_IDENTIFIER:
    case AST_LITERAL:
        /* listové uzly – nemajú deti */
        break;

    default:
        break;
    }
}
