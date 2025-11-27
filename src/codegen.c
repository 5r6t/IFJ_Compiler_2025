//////////////////////////////////////////////
// filename: codegen.c                      //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "codegen.h"
#include "common.h"
#include "ast.h"
#include <stdbool.h>

#define NAME_BUF 256
TAClist tac = {NULL, NULL};

/** TO-DO
 * scopedepth ~~ we'll see
* @brief int scopeDepth - indicating depth level for differentiating
    between global, local and temporary variables
*/

// used for printing NAMES of operands, see codegen.h
static const char *tac_opcode_name(OpCode op)
{
    switch (op)
    {
#define OPCODE_CASE(name) \
    case name:            \
        return #name;
        TAC_OPCODE_LIST(OPCODE_CASE)
#undef OPCODE_CASE
    default:
        return "UNKNOWN";
    }
}

void tac_print_list_state(const char *label, const TAClist *list)
{
    const char *tag = label ? label : "TAC list";
    size_t count = 0;
    const TACnode *curr = (list ? list->head : NULL);
    // count number of nodes
    while (curr)
    {
        ++count;
        curr = curr->next;
    }

    printf("\n== TAC LIST :: %s ==\n", tag);
    printf("\tsize  : %zu\n", count);
    printf("\tempty : %s\n", tac_list_is_empty(list) ? "yes" : "no");
    printf("\thead  : %s\n",
           (list && list->head) ? tac_opcode_name(list->head->instr) : "NULL");
    printf("\ttail  : %s\n",
           (list && list->tail) ? tac_opcode_name(list->tail->instr) : "NULL");
}

void tac_print_node(const char *label, const TACnode *node)
{
    const char *tag = label ? label : "TAC node";
    printf("\n-- TAC NODE :: %s --\n", tag);
    if (node == NULL)
    {
        printf("  (null)\n");
        return;
    }

    printf("\tinstr : %s (%d)\n", tac_opcode_name(node->instr), node->instr);
    printf("\targs  : %s | %s | %s\n",
           node->a1 ? node->a1 : "NULL",
           node->a2 ? node->a2 : "NULL",
           node->a3 ? node->a3 : "NULL");
}
/// @brief
/// @param list
void tac_list_init(TAClist *list)
{
    if (list == NULL)
    {
        return;
    }
    list->head = NULL;
    list->tail = NULL;
}

/// @brief
/// @param list
/// @return
bool tac_list_is_empty(const TAClist *list)
{
    return (list == NULL) || (list->head == NULL);
}

/// @brief
/// @param list
/// @param instr
/// @param a1
/// @param a2
/// @param a3
/// @return
TACnode *tac_list_append(TAClist *list, OpCode instr, const char *a1, const char *a2, const char *a3)
{
    if (list == NULL)
    {
        return NULL;
    }

    TACnode *n = malloc(sizeof(TACnode));
    if (n == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    n->instr = instr;
    n->a1 = a1 ? my_strdup(a1) : NULL;
    n->a2 = a2 ? my_strdup(a2) : NULL;
    n->a3 = a3 ? my_strdup(a3) : NULL;
    n->next = NULL;
    n->prev = list->tail;

    if (list->tail)
    {
        list->tail->next = n;
    }
    else
    {
        list->head = n;
    }
    list->tail = n;

    return n;
}

/// @brief
/// @param list
/// @return
TACnode *tac_list_pop_front(TAClist *list)
{
    if (list == NULL || list->head == NULL)
    {
        return NULL;
    }

    TACnode *node = list->head;
    list->head = node->next;
    if (list->head)
    {
        list->head->prev = NULL;
    }
    else
    {
        list->tail = NULL;
    }
    node->next = NULL;
    node->prev = NULL;

    return node;
}

/// @brief
/// @param list
/// @param node
void tac_list_remove(TAClist *list, TACnode *node)
{
    if (list == NULL || node == NULL)
    {
        return;
    }

    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = node->next;
    }

    if (node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
}

/// @brief
/// @param node
void tac_node_free(TACnode *node)
{
    if (node == NULL)
    {
        return;
    }

    free(node->a1);
    free(node->a2);
    free(node->a3);
    free(node);
}

/// @brief Clears the entire list and resets head/tail
/// @param list
void tac_list_clear(TAClist *list)
{
    if (list == NULL)
    {
        return;
    }

    TACnode *curr = list->head;
    while (curr)
    {
        TACnode *next = curr->next;
        tac_node_free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
}

/// @brief Function that appends the next TAC intruction to double-linked list,
///        abstracting from using global TAClist structure.
/// @param instr
/// @param a1
/// @param a2
/// @param a3
/// @return
TACnode *tac_append(OpCode instr, char *a1, char *a2, char *a3)
{
    return tac_list_append(&tac, instr, a1, a2, a3);
}

///////////////////////////////////
// ---- Strings for Operands ----
///////////////////////////////////

/// @brief Creates string in "$x" format, x is a string for functions
/// @param name
/// @return
char *fnc_label(char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "$%s", name);
    return my_strdup(buf);
}

/* returns string for a global variable */
char *var_gf(const char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "GF@%s", name);
    return my_strdup(buf);
}
/* returns string for a local variable */
char *var_lf(const char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LF@%s", name);
    return my_strdup(buf);
}

/// @brief global counter for temporary TF variables
/// @param num
/// @return
char *new_tf(int num)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "TF@%%%d", num);
    return my_strdup(buf);
}

/// @brief
/// @param name
/// @return
/// TODO: Complete
char *fnc_name(const char *name)
{
    char *ret = my_strdup(name);
    return ret;
}

///////////////////////////////////
// ---- Data types conversions ----
///////////////////////////////////

char *lit_int(long long x)
{
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "int@%lld", x);
    return my_strdup(buf);
}
char *lit_bool(bool x)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "bool@%s", x ? "true" : "false");
    return my_strdup(buf);
}
char *lit_float(double x)
{
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "float@%a", x);
    return my_strdup(buf);
}
char *lit_string(const char *x)
{
    char buf[4096];
    char *out = buf;

    while (*x)
    {
        unsigned char c = (unsigned char)*x++;

        // catch escape characters 0-32, 35 (#) and 92 (\)
        if (c <= 32 || c == '#' || c == '\\')
        {
            out += sprintf(out, "\\%03u", c);
        }
        else
        {
            *out++ = c;
        }
    }
    *out = '\0';

    char final[4200];
    snprintf(final, sizeof(final), "string@%s", buf);
    return my_strdup(final);
}
char *lit_nil()
{
    return my_strdup("nil@nil");
}

/// @brief function that determines if a variable is global or local
/// @param name
/// @param scope_depth
/// @return
char *var_gf_or_lf(char *name, int scope_depth)
{
    char buf[64];

    if (scope_depth == 0)
    {
        snprintf(buf, sizeof(buf), "GF@%s", name);
    }
    else
    {
        snprintf(buf, sizeof(buf), "LF@%s", name);
    }
    return my_strdup(buf);
}

///////////////////////////////////
// ---- Temporary variables
///////////////////////////////////

static int tmp_counter = 0;
static int label_counter = 0;

/// @brief Create string in format: TF@x, where x is a number
/// @return 
static char *new_tmp()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "TF@%%%d", tmp_counter++);
    return my_strdup(buf);
}

/// @brief Create labels in style: <prefix>x, where x is a counter
static char *new_label(const char *prefix)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "%s%d", prefix, label_counter++);
    return my_strdup(buf);
}

///////////////////////////////////
// ---- AST Nodes Codegen ----
///////////////////////////////////

/// @brief 
/// @param node 
void gen_block(ASTptr node)
{
    for (int i = 0; i < node->block.stmtCount; i++) {
        gen_stmt(node->block.stmt[i]);
    }
}

/// @brief function that appends instructions for a definition of a function to a list
/// @param node
void gen_func_def(ASTptr node)
{
    // check if functions is main
    bool is_main = strcmp(node->func.name, "main") == 0;
    // label is either $$main or $func_name
    char *label = is_main ? my_strdup("$$main") : fnc_label(node->func.name);
    tac_append(LABEL, label, NULL, NULL);
    
    
    if (is_main == true)
    {
        tac_append(CREATEFRAME, NULL, NULL, NULL);
    }
    tac_append(PUSHFRAME, NULL, NULL, NULL);

    // create a dedicated entry label for the generated function body
    char *body_entry_label = new_label("$body$");
    tac_append(LABEL, body_entry_label, NULL, NULL);

    // handle parameters
    for (int i = 0; i < node->func.paramCount; i++)
    {
        // ASTptr argNode = node->call.args[i]; forgot what i wanted
        char *local_param = var_lf(node->func.paramNames[i]);

        tac_append(DEFVAR, local_param, NULL, NULL);
        char *local_var = var_lf(node->func.paramNames[i]); // var_lf expects a name string

        tac_append(MOVE, local_param, local_var, NULL);
    }

    // TODO: handle body
    gen_block(node->func.body);

    tac_append(POPFRAME, NULL, NULL, NULL);
    if (is_main == false)
    {
        tac_append(RETURN, NULL, NULL, NULL);
    }
}

/// @brief
/// @param node
void gen_func_call(ASTptr node)
{
    tac_append(CREATEFRAME, NULL, NULL, NULL);

    // for every function argument
    for (int i = 0; i < node->call.argCount; i++)
    {
        int REPLACE_LATER = 0;
        char *temp_frame = new_tf(REPLACE_LATER);
        tac_append(DEFVAR, temp_frame, NULL, NULL);

        ASTptr argNode = node->call.args[i];
        char *fnc_param;
        // argument is a variable
        if (argNode->type == AST_IDENTIFIER)
        {
            fnc_param = gen_identifier(argNode);
        }
        // argument is a literal
        else if (argNode->type == AST_LITERAL)
        {
            fnc_param = gen_literal(argNode);
        }

        tac_append(MOVE, temp_frame, fnc_param, NULL);
    }
    char *name = fnc_name(node->call.funcName);
    tac_append(CALL, name, NULL, NULL);
}

/// @brief declaration of a variable
/// @param node
/// @param scope_depth
void gen_var_decl(ASTptr node, int scope_depth)
{
    char *var = var_gf_or_lf(node->var_decl.varName, scope_depth);
    tac_append(DEFVAR, var, NULL, NULL);
}

/// @brief
/// @param node
void gen_assign_stmt(ASTptr node)
{
    char *target;

    if(node->assign_stmt.asType == TARGET_GLOBAL) {
        target = var_gf(node->assign_stmt.targetName);
    } else {
        target = var_lf(node->assign_stmt.targetName);
    }

    char *expr = gen_expr(node->assign_stmt.expr);
    tac_append(MOVE, target, expr, NULL);
    return;
}

/// @brief
/// @param node
/// @param scopeDepth
void gen_while_stmt(ASTptr node, int scopeDepth) 
{
    (void)node;
    (void)scopeDepth;
    return;
}

void gen_if_stmt(ASTptr node, int scopeDepth) 
{   
    (void)node;
    (void)scopeDepth;
    return;
}

void gen_return_stmt(ASTptr node, int scopeDepth) 
{
    (void)node;
    (void)scopeDepth;
    return;
}

char *gen_binop_add(char *res, char *l, char *r)
{
    tac_append(ADD, res, l, r);
    return res;
}

char *gen_binop_sub(char *res, char *l, char *r)
{
    tac_append(SUB, res, l, r);
    return res;
}

char *gen_binop_mul(char *res, char *l, char *r)
{
    tac_append(MUL, res, l, r);
    return res;
}

char *gen_binop_div(char *res, char *l, char *r)
{
    tac_append(DIV, res, l, r);
    return res;
}

/// @brief function that converts identifiers to a desired format
/// @param node
/// @return
char *gen_identifier(ASTptr node)
{
    // global
    if (node->identifier.idType == ID_GLOBAL) {
        return var_gf(node->identifier.name); // GF@__blabla
    }
    // local
    if (node->identifier.idType == ID_LOCAL) {
        return var_lf(node->identifier.name); // LF@bla
    }

    // getter
    if (node->identifier.idType == ID_GETTER) {
        char *tmp = new_tmp();
        tac_append(DEFVAR, tmp, NULL, NULL);

        // Call getter function
        char *fn = fnc_name(node->identifier.name);
        tac_append(CALL, fn, NULL, NULL);

        // TODO probably? adjust later
        // current idea -- assuming getter is accessible directly
        tac_append(MOVE, tmp, "TF@%0", NULL);
        return tmp;
    }
    fprintf(stderr, "CODEGEN -> Error: in gen_identifier");
    exit(1);
}

/// @brief function that converts literals to a desired format
/// @param node
/// @return
char *gen_literal(ASTptr node)
{
    switch (node->literal.liType)
    {
        char *r;
    case LIT_NULL:
        r = lit_nil();
        return r;

        /* case LIT_BOOL:
            bool b = node->literal.bool;
            char *r = lit_bool(b);
            return r;
        */

    case LIT_NUMBER:
    {
        double v = node->literal.num;

        long long iv = (long long)v;
        if ((double)iv == v)
        {
            char *r = lit_int(iv);
            return r;
        }
        else
        {
            r = lit_float(v);
            return r;
        }
    }

    case LIT_STRING:
        r = lit_string(node->literal.str);
        return r;

    case LIT_LOCAL_ID:
        return var_lf(node->literal.str);
    case LIT_GLOBAL_ID:
        return var_gf(node->literal.str);

    default:
        return NULL;
    }
}

/// @brief 
/// @param node 
/// @return 
char *gen_func_call_expr(ASTptr node)
{
    // Create temp for return value
    char *tmp = new_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);

    // Prepare call frame
    tac_append(CREATEFRAME, NULL, NULL, NULL);

    // For each argument
    for (int i = 0; i < node->call.argCount; i++)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "TF@%%%d", i);
        tac_append(DEFVAR, my_strdup(buf), NULL, NULL);

        char *arg = gen_expr(node->call.args[i]);
        tac_append(MOVE, my_strdup(buf), arg, NULL);
    }

    char *name = fnc_name(node->call.funcName);
    tac_append(CALL, name, NULL, NULL);

    // copy TF@%0 into our local temp
    tac_append(MOVE, tmp, "TF@%0", NULL);

    return tmp;
}


/// @brief
/// @param node
char *gen_binop(ASTptr node)
{
    char *left = gen_expr(node->binop.left);
    char *right = gen_expr(node->binop.right);

    char *res = new_tmp();
    tac_append(DEFVAR, res, NULL, NULL);
    
    ;
    switch (node->binop.opType)
    {
    case BINOP_ADD:
        //return gen_binop_add(res,left,right);
    case BINOP_SUB:
        //return gen_binop_sub(res,left,right);
    case BINOP_MUL:
        //return gen_binop_mul(res,left,right);
    case BINOP_DIV:
        //return gen_binop_div(res,left,right);

    case BINOP_LT:
    case BINOP_GT:
    case BINOP_LTE:
    case BINOP_GTE:
        //return gen_binop_rel(res, left, right, node->binop.opType);

    case BINOP_EQ:
    case BINOP_NEQ:
        //return gen_binop_eq(res, left, right, node->binop.opType);
    
    case BINOP_IS:
        //return gen_binop_is(res,left,right,node->binop.opType);
        fprintf(stderr, "RelOps and more not implemented yet\nLEFT:%s\nRIGHT:%s", left, right);
        exit(ERR_INTERNAL);
    default:
        break;
    }
    fprintf(stderr, "generate binary operand failed");
    exit(0);
}

/// @brief 
/// @param node 
void gen_stmt(ASTptr node) {
    switch (node->type)
    {
    case AST_VAR_DECL:
        gen_var_decl(node, 1);
        break;
    case AST_ASSIGN_STMT:
        gen_assign_stmt(node);
        break;
    case AST_RETURN_STMT:
        // scope depth 1 for now
        gen_return_stmt(node, 1);
        break;
    case AST_IF_STMT:
        // scope depth 1 for now
        gen_if_stmt(node, 1);
        break;
    case AST_WHILE_STMT:
        gen_while_stmt(node, 1);
        break;
    case AST_FUNC_CALL:
        gen_func_call(node);
        break;
    
    default:
        DEBUG_PRINT("Unhandled Statement Type %d\n", node->type);
        break;
    }
}

/// @brief appends prolog of a program to a list, iterates over gen_func_def
/// @param node
void gen_program(ASTptr node)
{
    // header
    printf(".IFJcode25\n");
    tac_append(JUMP, "$$main", NULL, NULL);

    for (int i = 0; i < node->program.funcsCount; i++) {
        gen_func_def(node->program.funcs[i]);
    }

    return;
}

///////////////////////////////////
// ---- Traversal and Output
///////////////////////////////////

/// @brief 
/// @param node 
/// @return 
char *gen_expr(ASTptr node)
{
    switch (node->type)
    {
        case AST_LITERAL:
            return gen_literal(node);

        case AST_IDENTIFIER:
            return gen_identifier(node);

        case AST_BINOP:
            return gen_binop(node);

        case AST_FUNC_CALL:
            return gen_func_call_expr(node);

        default:
            fprintf(stderr, "gen_expr: unsupported AST node -> type %d\n", node->type);
            exit(ERR_INTERNAL);
    }
}

/// @brief prints the entire list to standard output
/// @param
void print_tac(void)
{
    for (const TACnode *curr = tac.head; curr; curr = curr->next)
    {
        printf("%s", tac_opcode_name(curr->instr));
        if (curr->a1)
            printf(" %s", curr->a1);
        if (curr->a2)
            printf(" %s", curr->a2);
        if (curr->a3)
            printf(" %s", curr->a3);
        putchar('\n');
    }
}

/// @brief Entry Point of the Codegen Part
/// @param tree
void generate(ASTptr tree)
{
    if (!tree) exit(ERR_INTERNAL);

    tac_list_init(&tac);

    gen_program(tree);

    /* OUTPUT -- no optimalizations */
    print_tac();
}
