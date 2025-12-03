//////////////////////////////////////////////
// filename: codegen.c                      //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//////////////////////////////////////////////

/** ==== TO-DO ====
// --- Runtime Error Codes ---
#define ERR_RUNTIME_ARG 25  // Runtime: invalid builtin parameter type
#define ERR_RUNTIME_TYPE 26 // Runtime: type mismatch in expr at runtime

HELPFUL NOTES:
- use Temporary Frame for variables (CREATEFRAME destroys old TF and it's vars)
- PUSHFRAME creates LF out out TF, keeps previous LF preserved if exists
*/

#include "codegen.h"
#include "common.h"
#include "ast.h"
#include <stdbool.h>

#define NAME_BUF 256
#define RUNTIME

TAClist tac = {NULL, NULL};

static int scope_depth = 0;

////////////////////////////////////////////////////////////////////

typedef struct
{
    char *name;
    int depth;
} PendingLocal;

static PendingLocal pending_locals[512];
static int __pending_cnt = 0;

static void pending_reset(void)
{
    for (int i = 0; i < __pending_cnt; ++i)
    {
        free(pending_locals[i].name);
    }
    __pending_cnt = 0;
}

static void pending_add(const char *name, int depth)
{
    pending_locals[__pending_cnt].name = my_strdup(name);
    pending_locals[__pending_cnt].depth = depth;
    __pending_cnt++;
}

static void emit_pending_locals(void)
{
    for (int i = 0; i < __pending_cnt; ++i)
    {
        char *lf = var_lf_at_depth(pending_locals[i].name,
                                   pending_locals[i].depth);
        tac_append(DEFVAR, lf, NULL, NULL);
    }
    pending_reset();
}

static void collect_locals(ASTptr node, int depth)
{
    if (!node)
        return;
    switch (node->type)
    {
    case AST_BLOCK:
        depth++;
        for (int i = 0; i < node->block.stmtCount; ++i)
        {
            collect_locals(node->block.stmt[i], depth);
        }
        break;
    case AST_VAR_DECL:
        pending_add(node->var_decl.varName, depth);
        break;
    case AST_IF_STMT:
        collect_locals(node->ifstmt.then, depth);
        collect_locals(node->ifstmt.elsestmt, depth);
        break;
    case AST_WHILE_STMT:
        collect_locals(node->while_stmt.body, depth);
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////////////

typedef struct
{
    char *name;
    int depth;
} LocalBinding;

static LocalBinding locals[512];
static int __local_cnt = 0;
static void locals_reset(void) { __local_cnt = 0; }
static void locals_pop_to_depth(int depth)
{
    while (__local_cnt > 0 && locals[__local_cnt - 1].depth > depth)
    {
        free(locals[--__local_cnt].name);
    }
}
static void locals_add(const char *name)
{
    locals[__local_cnt].name = my_strdup(name);
    locals[__local_cnt].depth = scope_depth;
    __local_cnt++;
}
static int locals_lookup_depth(const char *name)
{
    for (int i = __local_cnt - 1; i >= 0; --i)
    {
        if (strcmp(locals[i].name, name) == 0)
            return locals[i].depth;
    }
    return -1;
}
////////////////////////////////////////////////////////////////////
// track whether handled function should return a value
static bool __has_explicit_ret = false;
// track whether main function is being handled
static bool __is_main = false;
////////////////////////////////////////////////////////////////////
// track globals to avoid reinitializing
static const char *def_globs[256];
static int def_glob_cnt = 0;
static void is_def_glob(const char *name)
{
    for (int i = 0; i < def_glob_cnt; i++)
    {
        if (strcmp(def_globs[i], name) == 0)
            return;
    }
    def_globs[def_glob_cnt++] = my_strdup(name);
    tac_list_replace_head(&tac, MOVE, name, "nil@nil", NULL); 
    tac_list_replace_head(&tac, DEFVAR, name, NULL, NULL); // becomes head

    return;
}
////////////////////////////////////////////////////////////////////

void hoist_defvars(TACnode *start_label, TACnode *end_label)
{
    if (!start_label || !end_label)
        return;

    enum
    {
        MAX_HOIST = 512
    };
    TACnode *to_move[MAX_HOIST];
    int count = 0;

    for (TACnode *curr = start_label->next; curr && curr != end_label;)
    {
        TACnode *next = curr->next;
        if (curr->instr == DEFVAR && count < MAX_HOIST)
        {
            tac_list_remove(&tac, curr);
            curr->next = curr->prev = NULL;
            to_move[count++] = curr;
        }
        curr = next;
    }

    for (int i = 0; i < count; ++i)
    {
        insert_before_node(start_label, to_move[i]);
    }
}


////////////////////////////////////////////////////////////////////
#define EMIT_ARG_EXIT()                         \
    do                                          \
    {                                           \
        tac_append(EXIT, "int@25", NULL, NULL); \
    } while (0)

#define EMIT_TYPE_EXIT()                        \
    do                                          \
    {                                           \
        tac_append(EXIT, "int@26", NULL, NULL); \
    } while (0)

// macro to avoid writing NULL into params each time while
// creating a label
#define EMIT_LABEL(label_name)                     \
    do                                             \
    {                                              \
        tac_append(LABEL, label_name, NULL, NULL); \
    } while (0)

#define EMIT_RETURN()                         \
    do                                        \
    {                                         \
        tac_append(RETURN, NULL, NULL, NULL); \
    } while (0)

#define EMIT_POPFRAME()                         \
    do                                          \
    {                                           \
        tac_append(POPFRAME, NULL, NULL, NULL); \
    } while (0)

#define EMIT_CREATEFRAME()                         \
    do                                             \
    {                                              \
        tac_append(CREATEFRAME, NULL, NULL, NULL); \
    } while (0)

#define EMIT_DEFVAR(var_name)                     \
    do                                            \
    {                                             \
        tac_append(DEFVAR, var_name, NULL, NULL); \
    } while (0)

///////////////////////////////////
// ---- Strings for Operands ----
///////////////////////////////////

/// @brief Creates string in "$x" format, x is a string for functions
char *fnc_label(char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "$%s", name);
    return my_strdup(buf);
}

/// @brief eturns string for a global variable
char *var_gf(const char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "GF@%s", name);
    return my_strdup(buf);
}

/// @brief
/// TODO: Complete
char *fnc_name(const char *name)
{
    char *ret = my_strdup(name);
    return ret;
}

/// @brief Create labels in style: <prefix>x, where x is a counter
static char *new_label(const char *prefix)
{
    char buf[128];
    static int label_counter = 0;

    snprintf(buf, sizeof(buf), "%s%d", prefix, label_counter++);
    return my_strdup(buf);
}

///////////////////////////i////////
// ---- Data types conversions ----
///////////////////////////////////

char *lit_int(long long x)
{
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "int@%lld", x);
    return my_strdup(buf);
}

/// @brief For extensions, not used rn
char *lit_bool(bool x)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "bool@%s", x ? "true" : "false");
    return my_strdup(buf);
}

/// @brief
char *lit_float(double x)
{
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "float@%a", x);
    return my_strdup(buf);
}

/// @brief
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

/// @brief returns null in correct Ifjcode format
char *lit_nil() { return my_strdup("nil@nil"); }

///////////////////////////////////
// ---- Variables
///////////////////////////////////

/// @brief returns string for temporary TF variables
char *new_tf(int num)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "TF@%%%d", num);
    return my_strdup(buf);
}

/// @brief Create string in format: TF@x, where x is a number
static char *new_tf_tmp()
{
    char buf[NAME_BUF];
    static int tf_counter = 0;

    snprintf(buf, sizeof(buf), "TF@%%%d", tf_counter++);
    return my_strdup(buf);
}

/// @brief string for a local variable
char *var_lf(const char *name)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LF@%s", name);
    return my_strdup(buf);
}

/// @brief Create string in format: LF@x, where x is a number
static char *new_lf_tmp()
{
    char buf[NAME_BUF];
    static int lf_counter = 0;

    snprintf(buf, sizeof(buf), "LF@%%%d", lf_counter++);
    return my_strdup(buf);
}

/// @brief  returns string for a local variable with depth info
char *var_lf_at_depth(const char *name, int depth)
{
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LF@%s$%d", name, depth);
    return my_strdup(buf);
}

///////////////////////////////////
// ---- Built in handling
///////////////////////////////////

static void gen_builtin_substring(char *string, char *i_num, char *j_num, char *result)
{
    // temps
    char *tmp = new_tf_tmp();
    char *len = new_tf_tmp();
    char *k = new_tf_tmp();
    char *i_type = new_tf_tmp();
    char *j_type = new_tf_tmp();

    char *L_return_nil = new_label("$substr_ret_nil_");
    char *L_loop = new_label("$substr_loop_");
    char *L_loop_end = new_label("$substr_loop_end_");
    char *L_bounds_ok = new_label("$substr_bounds_ok_");
    char *L_end = new_label("$substr_end_");
    char *L_error = new_label("$substr_err_");

    EMIT_DEFVAR(tmp);
    EMIT_DEFVAR(len);
    EMIT_DEFVAR(k);
    EMIT_DEFVAR(i_type);
    EMIT_DEFVAR(j_type);
    // ----- i and j must be int -----
    tac_append(TYPE, i_type, i_num, NULL);
    tac_append(TYPE, j_type, j_num, NULL);
    // check tediously all types
    tac_append(JUMPIFEQ, L_error, i_type, "string@string");
    tac_append(JUMPIFEQ, L_error, i_type, "string@nil"); // if only right is string - error
    tac_append(JUMPIFEQ, L_error, i_type, "string@float");
    tac_append(JUMPIFEQ, L_error, j_type, "string@string");
    tac_append(JUMPIFEQ, L_error, j_type, "string@nil");    // if only right is string - error
    tac_append(JUMPIFEQ, L_error, j_type, "string@float");

    // compute len(s)
    tac_append(STRLEN, len, string, NULL);

    // i < 0 - nil
    tac_append(LT, tmp, i_num, "int@0");
    tac_append(JUMPIFEQ, L_return_nil, tmp, "bool@true");
    // j < 0 - nil
    tac_append(LT, tmp, j_num, "int@0");
    tac_append(JUMPIFEQ, L_return_nil, tmp, "bool@true");

    // i > j - nil
    tac_append(GT, tmp, i_num, j_num);
    tac_append(JUMPIFEQ, L_return_nil, tmp, "bool@true");

    // i >= len => compute (i < len)
    tac_append(LT, tmp, i_num, len);
    // if tmp == true - ok, continue
    tac_append(JUMPIFEQ, L_bounds_ok, tmp, "bool@true");
    // else - i >= len
    tac_append(JUMP, L_return_nil, NULL, NULL);

    EMIT_LABEL(L_bounds_ok);
    // j > len
    tac_append(GT, tmp, j_num, len);
    tac_append(JUMPIFEQ, L_return_nil, tmp, "bool@true");

    // res = ""
    tac_append(MOVE, result, "string@", NULL);

    // k = i
    tac_append(MOVE, k, i_num, NULL);

    EMIT_LABEL(L_loop);

    // if k >= j - end
    // check (k < j)
    tac_append(LT, tmp, k, j_num);
    tac_append(JUMPIFEQ, L_loop_end, tmp, "bool@false");
    // IF tmp is false - !(k < j) - k >= j

    // tmp = s[k]
    tac_append(GETCHAR, tmp, string, k);
    // result_tmp += tmp
    tac_append(CONCAT, result, result, tmp);

    // k = k + 1
    tac_append(ADD, k, k, "int@1");
    tac_append(JUMP, L_loop, NULL, NULL);

    EMIT_LABEL(L_loop_end);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_return_nil);
    tac_append(MOVE, result, "nil@nil", NULL);

    EMIT_LABEL(L_end);
}

static void gen_builtin_strcmp(ASTptr call, char *result_tmp)
{
    char *s1 = gen_expr(call->call.args[0]);
    char *s2 = gen_expr(call->call.args[1]);

    // temps
    char *i = new_tf_tmp();
    EMIT_DEFVAR(i);
    char *c1 = new_tf_tmp();
    EMIT_DEFVAR(c1);
    char *c2 = new_tf_tmp();
    EMIT_DEFVAR(c2);
    char *len1 = new_tf_tmp();
    EMIT_DEFVAR(len1);
    char *len2 = new_tf_tmp();
    EMIT_DEFVAR(len2);
    char *tmp = new_tf_tmp();
    EMIT_DEFVAR(tmp);

    char *LBL_loop = new_label("$strcmp_loop_");
    char *LBL_loop_end = new_label("$strcmp_end_");
    char *L_end = new_label("$strcmp_ret_");
    char *L_after_diff = new_label("$strcmp_after_diff_");

    // len1 = length(s1)
    tac_append(STRLEN, len1, s1, NULL);
    // len2 = length(s2)
    tac_append(STRLEN, len2, s2, NULL);

    // i = 0
    tac_append(MOVE, i, "int@0", NULL);

    // loop:
    tac_append(LABEL, LBL_loop, NULL, NULL);

    // if i >= len1 or i >= len2 - go to post-loop
    tac_append(LT, tmp, i, len1);
    tac_append(JUMPIFEQ, LBL_loop_end, tmp, "bool@false"); // !(i < len1)
    tac_append(LT, tmp, i, len2);
    tac_append(JUMPIFEQ, LBL_loop_end, tmp, "bool@false"); // !(i < len2)

    // c1 = ASCII of s1[i]
    tac_append(STRI2INT, c1, s1, i);
    // c2 = ASCII of s2[i]
    tac_append(STRI2INT, c2, s2, i);

    // if c1 == c2 - continue
    tac_append(EQ, tmp, c1, c2);
    tac_append(JUMPIFEQ, L_after_diff, tmp, "bool@false"); // if equal - skip diff calc

    // difference - result_tmp = c1 - c2
    tac_append(SUB, result_tmp, c1, c2);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_after_diff);
    // i = i + 1
    tac_append(ADD, i, i, "int@1");

    tac_append(JUMP, LBL_loop, NULL, NULL);

    // loop_end: one string ended or both equal so far
    tac_append(LABEL, LBL_loop_end, NULL, NULL);

    // compare lengths
    tac_append(SUB, result_tmp, len1, len2);

    EMIT_LABEL(L_end);
}

static void gen_builtin_str(ASTptr call, char *result_tmp)
{
    // argument evaluated first
    char *arg = gen_expr(call->call.args[0]);

    // temp for storing detected type
    char *t = new_tf_tmp();
    EMIT_DEFVAR(t);
    tac_append(TYPE, t, arg, NULL);

    // labels
    char *L_int = new_label("$str_int_");
    char *L_float = new_label("$str_float_");
    char *L_str = new_label("$str_str_");
    char *L_nil = new_label("$str_nil_");
    char *L_end = new_label("$str_end_");

    // type dispatch
    tac_append(JUMPIFEQ, L_int, t, "string@int");
    tac_append(JUMPIFEQ, L_float, t, "string@float");
    tac_append(JUMPIFEQ, L_str, t, "string@string");
    tac_append(JUMPIFEQ, L_nil, t, "string@nil");

    // (should not reach here unless uninitialized variable)
    // Move nil on fallback
    tac_append(MOVE, result_tmp, "string@nil", NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // ---- int branch ----
    tac_append(LABEL, L_int, NULL, NULL);
    tac_append(INT2STR, result_tmp, arg, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // ---- float branch ----
    tac_append(LABEL, L_float, NULL, NULL);
    tac_append(FLOAT2STR, result_tmp, arg, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // ---- string branch ----
    tac_append(LABEL, L_str, NULL, NULL);
    tac_append(MOVE, result_tmp, arg, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // ---- nil branch ----
    tac_append(LABEL, L_nil, NULL, NULL);
    tac_append(MOVE, result_tmp, "string@nil", NULL);

    // ---- end ----
    tac_append(LABEL, L_end, NULL, NULL);
    return;
}

static void gen_builtin_ord(char *string, char *num, char *result_tmp)
{
    char *tmp = new_tf_tmp();
    char *len = new_tf_tmp();
    EMIT_DEFVAR(tmp);
    EMIT_DEFVAR(len);
    // empty or out of index => return 0
    // err if num not int
    char *L_error = new_label("$ord_num_not_int_");
    char *L_return_zero = new_label("$ord_out_of_bounds_");
    char *L_end = new_label("$ord_end_");

    // num must be integer
    tac_append(ISINT, tmp, num, NULL);
    tac_append(JUMPIFNEQ, L_error, tmp, "bool@true");

    // compute length of string
    tac_append(STRLEN, len, string, NULL);

    // if num < 0
    tac_append(LT, tmp, num, "int@0");
    tac_append(JUMPIFEQ, L_return_zero, tmp, "bool@true");
    // if num >= len  (num < len must be true)
    tac_append(LT, tmp, num, len);
    tac_append(JUMPIFEQ, L_return_zero, tmp, "bool@false");
    // convert
    tac_append(STRI2INT, result_tmp, string, num);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_return_zero);
    tac_append(MOVE, result_tmp, "int@0", NULL);

    EMIT_LABEL(L_end);
}

static void gen_builtin_chr(char *num, char *result_tmp)
{
    char *tmp = new_tf_tmp();
    EMIT_DEFVAR(tmp);

    char *L_error = new_label("$chr_err_range_");
    char *L_end = new_label("$chr_end_");

    // must be int
    tac_append(ISINT, tmp, num, NULL);
    tac_append(JUMPIFNEQ, L_error, tmp, "bool@true");

    tac_append(INT2CHAR, result_tmp, num, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_ARG_EXIT();

    EMIT_LABEL(L_end);
}
/// TODO implement builtin checker/helper
static bool is_builtin(const char *func_name)
{
    static const char *arr_builtin_names[] = {
        "Ifj.read_str",
        "Ifj.read_num",
        "Ifj.write",
        "Ifj.floor",
        "Ifj.str",
        "Ifj.length",
        "Ifj.substring",
        "Ifj.strcmp",
        "Ifj.ord",
        "Ifj.chr",
        NULL};

    for (int i = 0; arr_builtin_names[i] != NULL; i++)
    {
        if (strcmp(arr_builtin_names[i], func_name) == 0)
        {
            return true;
        }
    }
    return false;
}

/// @brief built in function caller/builder
void gen_builtin_call(ASTptr call, char *result_tmp)
{
    const char *name = call->call.funcName;
    if (strcmp(name, "Ifj.read_str") == 0)
    {
        tac_append(READ, result_tmp, "string", NULL); // "string" ?? idk
        return;
    }

    if (strcmp(name, "Ifj.read_num") == 0)
    {
        tac_append(READ, result_tmp, "float", NULL);
        return;
    }

    if (strcmp(name, "Ifj.write") == 0)
    {
        // arity 1 expected
        char *arg = gen_expr(call->call.args[0]);
        tac_append(WRITE, arg, NULL, NULL);

        // Ifj.write returns null
        tac_append(MOVE, result_tmp, "nil@nil", NULL);
        return;
    }

    if (strcmp(name, "Ifj.floor") == 0)
    {
        char *arg = gen_expr(call->call.args[0]);
        char *tmp = new_lf_tmp();
        EMIT_DEFVAR(tmp);

        // convert float to int (floor semantics)
        tac_append(FLOAT2INT, tmp, arg, NULL);

        // store return valu
        tac_append(MOVE, result_tmp, tmp, NULL);
        return;
    }

    if (strcmp(name, "Ifj.str") == 0) // REDO
    {
        gen_builtin_str(call, result_tmp);
        return;
    }

    if (strcmp(name, "Ifj.length") == 0)
    {
        char *arg = gen_expr(call->call.args[0]);
        tac_append(STRLEN, result_tmp, arg, NULL);
        return;
    }
    if (strcmp(name, "Ifj.substring") == 0)
    {
        char *s = gen_expr(call->call.args[0]);
        char *i = gen_expr(call->call.args[1]);
        char *j = gen_expr(call->call.args[2]);
        gen_builtin_substring(s, i, j, result_tmp);
        return;
    }
    if (strcmp(name, "Ifj.strcmp") == 0)
    {
        gen_builtin_strcmp(call, result_tmp);
        return;
    }
    if (strcmp(name, "Ifj.ord") == 0)
    {
        char *string = gen_expr(call->call.args[0]);
        char *num = gen_expr(call->call.args[1]);
        gen_builtin_ord(string, num, result_tmp);
        return;
    }

    if (strcmp(name, "Ifj.chr") == 0)
    {
        char *num = gen_expr(call->call.args[0]);
        gen_builtin_chr(num, result_tmp);
        return;
    }
}

///////////////////////////////////
// ---- AST Nodes Codegen ----
///////////////////////////////////

/// @brief
void gen_block(ASTptr node)
{
    scope_depth++;

    for (int i = 0; i < node->block.stmtCount; i++)
    {
        gen_stmt(node->block.stmt[i]);
    }

    scope_depth--;
    locals_pop_to_depth(scope_depth);
}

/// @brief function that appends instructions for a definition of a function to a list
void gen_func_def(ASTptr node)
{
    locals_reset();
    pending_reset();
    // check if functions is main -- stored in global variable
    __is_main = (strcmp(node->func.name, "main") == 0) &&
                (node->func.paramCount == 0);

    // label is either $$main or $func_name
    char *label = __is_main ? my_strdup("$$main") : fnc_label(node->func.name);
    EMIT_LABEL(label);

    char *retval = "LF@%retval1";

    if (__is_main == true)
    {
        EMIT_CREATEFRAME();
        tac_append(PUSHFRAME, NULL, NULL, NULL);
        EMIT_DEFVAR(retval);
    }
    else
    {
        tac_append(PUSHFRAME, NULL, NULL, NULL);
        tac_append(MOVE, retval, "nil@nil", NULL);
    }
    EMIT_CREATEFRAME(); // prepare TF for temporaries

    if (__is_main == false)
    {
        // handle parameters
        for (int i = 0; i < node->func.paramCount; i++)
        {
            // ASTptr argNode = node->call.args[i]; forgot what i wanted
            char *local_param = var_lf_at_depth(node->func.paramNames[i], scope_depth);
            EMIT_DEFVAR(local_param);

            // keep track of locals
            locals_add(node->func.paramNames[i]);
            // todo incomplete scoped handling
            char srcbuf[NAME_BUF];
            snprintf(srcbuf, sizeof(srcbuf), "LF@%%%d", i + 1); // start with 1

            tac_append(MOVE, local_param, my_strdup(srcbuf), NULL);
        }
    }
    // Generate function body
    collect_locals(node->func.body, 0);
    emit_pending_locals();
    __has_explicit_ret = false;
    gen_block(node->func.body);

    // function epilogue
    if (!__has_explicit_ret)
    {
        EMIT_POPFRAME();
        if (!__is_main) // do not return on empty stack
        {
            EMIT_RETURN();
        }
        else
        {
            tac_append(EXIT, "int@0", NULL, NULL); // TODO add real return values
        }
    }
}

/// @brief
void gen_func_call(ASTptr node)
{
    char *fname = node->call.funcName;
    // --- BUILTIN CALL ---
    if (is_builtin(fname))
    {
        EMIT_CREATEFRAME();
        char *tmp = new_tf_tmp();
        EMIT_DEFVAR(tmp);
        gen_builtin_call(node, tmp);
        return;
    }

    // --- USER-DEFINED FUNCTION CALL ---
    EMIT_CREATEFRAME();

    for (int i = 0; i < node->call.argCount; i++)
    {
        char buf[NAME_BUF];
        snprintf(buf, sizeof(buf), "TF@%%%d", i + 1);
        EMIT_DEFVAR(buf);

        char *arg = gen_expr(node->call.args[i]);
        tac_append(MOVE, buf, arg, NULL);
    }
    char *func_label = fnc_label(fname);
    tac_append(CALL, func_label, NULL, NULL);
}

/// @brief
void gen_assign_stmt(ASTptr node)
{
    char *target;

    if (node->assign_stmt.asType == TARGET_GLOBAL)
    {
        target = var_gf(node->assign_stmt.targetName);
        is_def_glob(target);
    }
    else
    {
        int depth = locals_lookup_depth(node->assign_stmt.targetName);
        target = var_lf_at_depth(node->assign_stmt.targetName, depth);
    }

    char *expr = gen_expr(node->assign_stmt.expr);
    tac_append(MOVE, target, expr, NULL);
    return;
}

/// @brief result = bool value of expr (num, null, etc)
/// @note expects to be in TF
char *gen_eval_bool(char *value)
{
    char *t_type = new_tf_tmp();
    EMIT_DEFVAR(t_type);
    tac_append(TYPE, t_type, value, NULL);

    char *out = new_tf_tmp();
    EMIT_DEFVAR(out);

    char *L_bool = new_label("$truth_bool_");
    char *L_nil = new_label("$truth_nil_");
    char *L_else = new_label("$truth_else_");
    char *L_end = new_label("$truth_end_");

    // if type == bool
    tac_append(JUMPIFEQ, L_bool, t_type, "string@bool");

    // if null - false
    tac_append(JUMPIFEQ, L_nil, t_type, "string@nil");

    // else - true
    tac_append(JUMP, L_else, NULL, NULL);

    // bool branch: out = value
    EMIT_LABEL(L_bool);
    tac_append(MOVE, out, value, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // null branch: out = false
    EMIT_LABEL(L_nil);
    tac_append(MOVE, out, "bool@false", NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // else branch: true
    EMIT_LABEL(L_else);
    tac_append(MOVE, out, "bool@true", NULL);

    EMIT_LABEL(L_end);
    return out; // always bool
}

/// @brief
void gen_while_stmt(ASTptr node)
{
    char *lbl_start = new_label("$while_start_");
    char *lbl_end = new_label("$while_end_");

    EMIT_LABEL(lbl_start);
    TACnode *while_start = tac.tail;

    char *raw = gen_expr(node->while_stmt.cond);
    char *cond = gen_eval_bool(raw);

    // jump to end if condition if false
    tac_append(JUMPIFEQ, lbl_end, cond, "bool@false");
    gen_block(node->while_stmt.body);
    // return to condition
    tac_append(JUMP, lbl_start, NULL, NULL);
    // finish
    EMIT_LABEL(lbl_end);
    TACnode *while_end = tac.tail;
    hoist_defvars(while_start, while_end);
    return;
}

void gen_if_stmt(ASTptr node)
{
    char *lbl_else = new_label("$if_else_");
    char *lbl_end = new_label("$if_end_");
    // 1. Evaluate Condition
    char *raw = gen_expr(node->ifstmt.cond);
    // 1.1 Convert to bool
    char *cond = gen_eval_bool(raw);
    // 2. Go to else if cond is false
    tac_append(JUMPIFEQ, lbl_else, cond, "bool@false");
    // 3. IF block
    gen_block(node->ifstmt.then);
    // 4. Skip ELSE block
    tac_append(JUMP, lbl_end, NULL, NULL);
    EMIT_LABEL(lbl_else);
    // 5. ELSE block (if exists) // EVEN THOUGH EXTENSION NOT IMPLEMENTED
    if (node->ifstmt.elsestmt != NULL)
        gen_block(node->ifstmt.elsestmt);
    // 6. finish
    EMIT_LABEL(lbl_end);
}

void gen_return_stmt(ASTptr node)
{
    __has_explicit_ret = true;

    char *expr = gen_expr(node->return_stmt.expr);
    tac_append(MOVE, "LF@%retval1", expr, NULL);

    if (__is_main)
    {
        return;
    }

    EMIT_POPFRAME();
    EMIT_RETURN();
    return;
}

///////////////////////////////////
// ---- BINARY OPERATORS  ----
///////////////////////////////////
// string + string or num + num, but not num + string/null --> ERR 26
char *gen_binop_add(char *res, char *lo, char *ro)
{
    char *l = new_tf_tmp();
    char *r = new_tf_tmp();
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();

    char *L_str_ok = new_label("$add_str_");
    char *L_float = new_label("$in_float_L_");
    char *L_int = new_label("$in_int_L_");
    char *L_rnum_ok = new_label("add_rnum");
    char *L_error = new_label("$add_err_");
    char *L_end = new_label("$add_end_");

    EMIT_DEFVAR(l);
    EMIT_DEFVAR(r);
    tac_append(MOVE, l, lo, NULL);
    tac_append(MOVE, r, ro, NULL);
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    // preemptive check
    tac_append(JUMPIFEQ, L_error, t_r, "string@nil");

    // string + string ?
    tac_append(JUMPIFEQ, L_str_ok, t_l, "string@string");
    tac_append(JUMPIFEQ, L_error, t_r, "string@string"); // if only right is string - error

    // numeric + numeric ?
    // if L float or int -- ok jump
    tac_append(JUMPIFEQ, L_float, t_l, "string@float");
    tac_append(JUMPIFEQ, L_int, t_l, "string@int");

    // error (type mismatch)
    tac_append(JUMP, L_error, NULL, NULL);

    EMIT_LABEL(L_str_ok);
    tac_append(JUMPIFNEQ, L_error, t_r, "string@string"); // right not string
    tac_append(CONCAT, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_float);
    // if right is float too, then ADD
    tac_append(JUMPIFEQ, L_rnum_ok, t_r, "string@float");
    // else convert
    tac_append(INT2FLOAT, r, r, NULL);
    tac_append(JUMP, L_rnum_ok, NULL, NULL);

    EMIT_LABEL(L_int);
    // if right is int, then ADD
    tac_append(JUMPIFEQ, L_rnum_ok, t_r, "string@int");
    // else convert
    tac_append(INT2FLOAT, l, l, NULL);
    tac_append(JUMP, L_rnum_ok, NULL, NULL);

    EMIT_LABEL(L_rnum_ok);
    tac_append(ADD, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_end);
    return res;
}

char *gen_binop_sub(char *res, char *lo, char *ro)
{
    char *l = new_tf_tmp();
    char *r = new_tf_tmp();
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();

    char *L_ok = new_label("$sub_ok_");
    char *L_error = new_label("$sub_err_");
    char *L_end = new_label("$sub_end_");
    char *L_float = new_label("$in_float_L_");
    char *L_int = new_label("$in_int_L_");

    EMIT_DEFVAR(l);
    EMIT_DEFVAR(r);
    tac_append(MOVE, l, lo, NULL);
    tac_append(MOVE, r, ro, NULL);
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    // preemptive checks
    tac_append(JUMPIFEQ, L_error, t_l, "string@nil");
    tac_append(JUMPIFEQ, L_error, t_r, "string@nil");
    tac_append(JUMPIFEQ, L_error, t_l, "string@string");
    tac_append(JUMPIFEQ, L_error, t_r, "string@string");

    tac_append(JUMPIFEQ, L_float, t_l, "string@float");
    tac_append(JUMPIFEQ, L_int, t_l, "string@int");

    EMIT_LABEL(L_float);
    // if right is float too, then SUB
    tac_append(JUMPIFEQ, L_ok, t_r, "string@float");
    // else convert
    tac_append(INT2FLOAT, r, r, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_int);
    // if right is int, then SUB
    tac_append(JUMPIFEQ, L_ok, t_r, "string@int");
    // else convert
    tac_append(INT2FLOAT, l, l, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_ok);
    tac_append(SUB, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_end);
    return res;
}

char *gen_binop_mul(char *res, char *lo, char *ro)
{
    char *l = new_tf_tmp();
    char *r = new_tf_tmp();
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();
    char *r_tmp = new_tf_tmp();

    char *L_ok = new_label("$mul_ok_");
    char *L_str_ok = new_label("$add_str_");
    char *L_error = new_label("$mul_err_");
    char *L_end = new_label("$mul_end_");
    char *L_float = new_label("$in_float_L_");
    char *L_int = new_label("$in_int_L_");
    char *L_loop = new_label("$iter_loop_");

    EMIT_DEFVAR(l);
    EMIT_DEFVAR(r);
    tac_append(MOVE, l, lo, NULL);
    tac_append(MOVE, r, ro, NULL);
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);
    EMIT_DEFVAR(r_tmp);
    tac_append(MOVE, r_tmp, "string@", NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    // preemptive check
    tac_append(JUMPIFEQ, L_error, t_r, "string@nil");

    // string + int ?
    tac_append(JUMPIFEQ, L_str_ok, t_l, "string@string");
    tac_append(JUMPIFEQ, L_error, t_r, "string@string"); // right cannot be string

    // numeric + numeric ?
    // if L float or int -- ok jump
    tac_append(JUMPIFEQ, L_float, t_l, "string@float");
    tac_append(JUMPIFEQ, L_int, t_l, "string@int");

    // error (type mismatch)
    tac_append(JUMP, L_error, NULL, NULL);

    EMIT_LABEL(L_str_ok);
    tac_append(JUMPIFNEQ, L_error, t_r, "string@int"); // right not int

    EMIT_LABEL(L_loop);
    tac_append(CONCAT, r_tmp, r_tmp, l);
    tac_append(SUB, r, r, "int@1");            // r = r - 1 => r--
    tac_append(JUMPIFNEQ, L_loop, r, "int@0"); // until r is zero
    tac_append(MOVE, res, r_tmp, NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_float);
    // if right is float too, then ADD
    tac_append(JUMPIFEQ, L_ok, t_r, "string@float");
    // else convert
    tac_append(INT2FLOAT, r, r, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_int);
    // if right is int, then ADD
    tac_append(JUMPIFEQ, L_ok, t_r, "string@int");
    // else convert
    tac_append(INT2FLOAT, l, l, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_ok);
    tac_append(MUL, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_end);
    return res;
}

char *gen_binop_div(char *res, char *lo, char *ro)
{
    char *l = new_tf_tmp();
    char *r = new_tf_tmp();
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();

    char *L_ok = new_label("$div_ok_");
    char *L_error = new_label("$div_err_");
    char *L_end = new_label("$div_end_");
    char *L_float = new_label("$in_float_L_");
    char *L_int = new_label("$in_int_L_");

    EMIT_DEFVAR(l);
    EMIT_DEFVAR(r);
    tac_append(MOVE, l, lo, NULL);
    tac_append(MOVE, r, ro, NULL);
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    // preemptive checks
    tac_append(JUMPIFEQ, L_error, t_l, "string@nil");
    tac_append(JUMPIFEQ, L_error, t_r, "string@nil");
    tac_append(JUMPIFEQ, L_error, t_l, "string@string");
    tac_append(JUMPIFEQ, L_error, t_r, "string@string");

    tac_append(JUMPIFEQ, L_float, t_l, "string@float");
    tac_append(JUMPIFEQ, L_int, t_l, "string@int");

    EMIT_LABEL(L_float);
    // if right is float too, then SUB
    tac_append(JUMPIFEQ, L_ok, t_r, "string@float");
    // convert to float
    tac_append(INT2FLOAT, r, r, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_int);
    // convert left op to float
    tac_append(INT2FLOAT, l, l, NULL);
    // if right is int, then DIV
    tac_append(JUMPIFEQ, L_ok, t_r, "string@float");
    tac_append(INT2FLOAT, r, r, NULL);
    tac_append(JUMP, L_ok, NULL, NULL);

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_ok);
    tac_append(DIV, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_end);
    return res;
}

/// @brief  Handles <, >, <=, >= (NUM ONLY)
char *gen_binop_rel(char *res, char *l, char *r, BinOpType rel)
{
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_num = new_label("$rel_numok_");
    char *L_do = new_label("$rel_do_");
    char *L_false = new_label("$rel_false_");
    char *L_end = new_label("$rel_end_");
    char *L_err = new_label("$rel_err_");

    // left must be int/float
    tac_append(JUMPIFEQ, L_num, t_l, "string@int");
    tac_append(JUMPIFEQ, L_num, t_l, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    EMIT_LABEL(L_num);
    // right must be int/float
    tac_append(JUMPIFEQ, L_do, t_r, "string@int");
    tac_append(JUMPIFEQ, L_do, t_r, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    EMIT_LABEL(L_do);

    // perform comparison
    switch (rel)
    {
    case BINOP_LT:
        tac_append(LT, res, l, r);
        break;

    case BINOP_GT:
        tac_append(GT, res, l, r);
        break;

    case BINOP_LTE:
        // l <= r  ->  !(l > r)
        tac_append(GT, res, l, r);
        // invert
        tac_append(JUMPIFEQ, L_false, res, "bool@true");
        tac_append(MOVE, res, "bool@true", NULL);
        tac_append(JUMP, L_end, NULL, NULL);

        tac_append(LABEL, L_false, NULL, NULL);
        tac_append(MOVE, res, "bool@false", NULL);
        tac_append(JUMP, L_end, NULL, NULL);
        break;

    case BINOP_GTE:
        // l >= r -> !(l < r)
        tac_append(LT, res, l, r);
        // invert
        tac_append(JUMPIFEQ, L_false, res, "bool@true");
        tac_append(MOVE, res, "bool@true", NULL);
        tac_append(JUMP, L_end, NULL, NULL);

        tac_append(LABEL, L_false, NULL, NULL);
        tac_append(MOVE, res, "bool@false", NULL);
        tac_append(JUMP, L_end, NULL, NULL);
        break;

    default:
        // unreachable
        break;
    }

    // for < and >, fallthrough ends here:
    tac_append(JUMP, L_end, NULL, NULL);

    // type error
    EMIT_LABEL(L_err);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_end);
    return res;
}
// todo
char *gen_binop_eq(char *res, char *l, char *r, BinOpType eq)
{
    char *t_l = new_tf_tmp();
    char *t_r = new_tf_tmp();
    EMIT_DEFVAR(t_l);
    EMIT_DEFVAR(t_r);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_types_eq = new_label("$rel_tyeq_");
    char *L_null_null = new_label("$rel_nullnull_");
    char *L_false = new_label("$rel_false_");
    char *L_end = new_label("$rel_end_");

    // if types equal - go compare
    tac_append(EQ, res, t_l, t_r);
    tac_append(JUMPIFEQ, L_types_eq, res, "bool@true");

    // types differ - null == null special case?
    // if both null types?
    tac_append(JUMPIFEQ, L_null_null, t_l, "string@nil");
    tac_append(JUMP, L_false, NULL, NULL);

    tac_append(LABEL, L_null_null, NULL, NULL);
    tac_append(JUMPIFEQ, L_false, t_r, "string@nil");

    // both null - true
    tac_append(MOVE, res, "bool@true", NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    // types same
    tac_append(LABEL, L_types_eq, NULL, NULL);

    // if null == null already handled; if both null return true
    tac_append(JUMPIFEQ, L_null_null, t_l, "string@nil");

    // Compare using EQ
    tac_append(EQ, res, l, r);

    if (eq == BINOP_NEQ)
    {
        // invert boolean
        tac_append(JUMPIFEQ, L_false, res, "bool@true");
        tac_append(MOVE, res, "bool@true", NULL); // was false - true
        tac_append(JUMP, L_end, NULL, NULL);

        tac_append(LABEL, L_false, NULL, NULL);
        tac_append(MOVE, res, "bool@false", NULL);
        tac_append(JUMP, L_end, NULL, NULL);
    }

    tac_append(JUMP, L_end, NULL, NULL);

    // final false block for EQ
    tac_append(LABEL, L_false, NULL, NULL);
    tac_append(MOVE, res, "bool@false", NULL);
    tac_append(JUMP, L_end, NULL, NULL);

    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}

char *gen_binop_is(char *res, char *lo, TypeName targetType)
{
    char *l = new_tf_tmp();
    char *t_l = new_tf_tmp();

    char *L_true  = new_label("$correct_type_");
    char *L_false = new_label("$incorrect_type_");
    char *L_error = new_label("$is_err_");
    char *L_end   = new_label("$is_end");

    EMIT_DEFVAR(l);
    tac_append(MOVE, l, lo, NULL);
    EMIT_DEFVAR(t_l);

    tac_append(TYPE, t_l, l, NULL);

    if (targetType == TYPE_STRING)
    {
        tac_append(JUMPIFNEQ, L_false, t_l, "string@string");
        tac_append(JUMP, L_true, NULL, NULL);
    }
    else if (targetType == TYPE_NUMBER)
    {
        tac_append(JUMPIFEQ, L_false, t_l, "string@string");
        tac_append(JUMPIFEQ, L_false, t_l, "string@nil");
        tac_append(JUMP, L_true, NULL, NULL);
    }
    else if (targetType == TYPE_NULL)
    {
        tac_append(JUMPIFNEQ, L_false, t_l, "string@nil");
        tac_append(JUMP, L_true, NULL, NULL);
    }

    EMIT_LABEL(L_error);
    EMIT_TYPE_EXIT();

    EMIT_LABEL(L_false);
        tac_append(MOVE, res, "bool@false", NULL);
        tac_append(JUMP, L_end, NULL, NULL);

    EMIT_LABEL(L_true);
        tac_append(MOVE, res, "bool@true", NULL);

    EMIT_LABEL(L_end);
    return res;
}

/// @brief
char *gen_binop(ASTptr node)
{
    char *left = gen_expr(node->binop.left);
    char *right = NULL;
    if (node->binop.opType != BINOP_IS)
    {
        right = gen_expr(node->binop.right);
    }

    char *res = new_tf_tmp();
    EMIT_DEFVAR(res);

    switch (node->binop.opType)
    {
    case BINOP_ADD:
        return gen_binop_add(res, left, right);
    case BINOP_SUB:
        return gen_binop_sub(res, left, right);
    case BINOP_MUL:
        return gen_binop_mul(res, left, right);
    case BINOP_DIV:
        return gen_binop_div(res, left, right);

    case BINOP_LT:
    case BINOP_GT:
    case BINOP_LTE:
    case BINOP_GTE:
        return gen_binop_rel(res, left, right, node->binop.opType);

    case BINOP_EQ:
    case BINOP_NEQ:
        return gen_binop_eq(res, left, right, node->binop.opType);

    case BINOP_IS:
        return gen_binop_is(res, left, node->binop.resultType);

    default:
        break;
    }
    fprintf(stderr, "generate binary operand failed");
    exit(0);
}

///////////////////////////////////
// ---- META FOR NOW  ----
///////////////////////////////////

/// @brief function that converts identifiers to a desired format
char *gen_identifier(ASTptr node)
{
    if (node->identifier.idType == ID_GLOBAL)
    {
        char *target=  var_gf(node->identifier.name);
        is_def_glob(target);
        return target;
    }

    if (node->identifier.idType == ID_LOCAL)
    {
        int depth = locals_lookup_depth(node->identifier.name);
        char buf[NAME_BUF];
        snprintf(buf, sizeof(buf), "LF@%s$%d", node->identifier.name, depth);
        return my_strdup(buf);
    }

    if (node->identifier.idType == ID_GETTER)
    {
        char *tmp = new_tf_tmp();
        EMIT_DEFVAR(tmp);

        // Call getter function
        char *fn = fnc_name(node->identifier.name);
        tac_append(CALL, fn, NULL, NULL);

        // TODO probably? adjust later
        // current idea -- assuming getter is accessible directly
        tac_append(MOVE, tmp, "TF@%0", NULL);
        return tmp;
    }
    fprintf(stderr, "CODEGEN -> Error: in gen_identifier");
    return NULL;
}
/// @brief function that converts literals to a desired format
char *gen_literal(ASTptr node)
{
    char *r;
    switch (node->literal.liType)
    {
    case LIT_NULL:
        r = lit_nil();
        return r;

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

    default:
        return NULL;
    }
}

/// @brief
void gen_stmt(ASTptr node)
{
    switch (node->type)
    {
    case AST_VAR_DECL:
    {

        // char *name = var_lf_at_depth(node->var_decl.varName, scope_depth);
        // tac_append(DEFVAR, name, NULL, NULL);
        locals_add(node->var_decl.varName);
        break;
    }
    case AST_ASSIGN_STMT:
        gen_assign_stmt(node);
        break;
    case AST_RETURN_STMT:
        gen_return_stmt(node);
        break;
    case AST_IF_STMT:
        gen_if_stmt(node);
        break;
    case AST_WHILE_STMT:
        gen_while_stmt(node);
        break;
    case AST_FUNC_CALL:
        gen_func_call(node);
        break;
    case AST_BLOCK:
        gen_block(node);
        break;

    default:
        DEBUG_PRINT("Unhandled Statement Type %d\n", node->type);
        break;
    }
}

/// @brief
char *gen_func_call_expr(ASTptr node)
{
    char *fname = node->call.funcName;
    // Create temp for return value
    EMIT_CREATEFRAME();
    char *tmp = new_tf_tmp();
    EMIT_DEFVAR(tmp);

    // Prepare call frame
    if (is_builtin(fname))
    {
        gen_builtin_call(node, tmp);
        return tmp;
    }

    // TF
    //EMIT_CREATEFRAME();
    // For each argument TF@1 .. TF@n
    for (int i = 0; i < node->call.argCount; i++)
    {
        char *tf = new_tf(i + 1);
        EMIT_DEFVAR(tf);

        char *arg = gen_expr(node->call.args[i]);
        tac_append(MOVE, my_strdup(tf), arg, NULL);
    }
    // return slot
    char *retval = "TF@%retval1";
    EMIT_DEFVAR(retval);
    // call
    char *func_lab = fnc_label(fname);
    tac_append(CALL, func_lab, NULL, NULL);
    // copy into local tmp
    tac_append(MOVE, tmp, retval, NULL);
    return tmp;
}

/// @brief
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
        DEBUG_PRINT("gen_expr: unsupported AST node -> type %d\n", node->type);
        return NULL; // reach only on errors
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Entry Point of the Codegen Part
void generate(ASTptr tree)
{
    if (!tree)
    {
        fprintf(stderr, "Deforestation in progress...");
        exit(ERR_INTERNAL);
    }

    tac_list_init(&tac);

    { /* BUILD LIST OF INSTRUCTIONS */
        // begin in main (param-less)
        tac_append(JUMP, "$$main", NULL, NULL);

        for (int i = 0; i < tree->program.funcsCount; i++)
        {
            gen_func_def(tree->program.funcs[i]);
        }
    }

    { /* OUTPUT */
        // print header
        printf(".IFJcode25\n");
        // print the rest of the program
        print_tac(&tac);
        // clear the list
        tac_list_clear(&tac);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
// ---- TAC Related ----
///////////////////////////////////

/// @brief prints the entire list to standard output
void print_tac(TAClist *list)
{
    for (const TACnode *curr = list->head; curr; curr = curr->next)
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

/// @brief DEBUG used for printing NAMES of operands, see codegen.h
const char *tac_opcode_name(OpCode op)
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

/// @brief
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
bool tac_list_is_empty(const TAClist *list)
{
    return (list == NULL) || (list->head == NULL);
}

/// @brief
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

/// @brief Replace head with new node, keeps head as next
TACnode *tac_list_replace_head(TAClist *list, OpCode instr, const char *a1, const char *a2, const char *a3)
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
    n->prev = NULL;
    n->next = list->head;

    if (list->head)
    {
        list->head->prev = n;
    }
    else
    {
        list->tail = n;
    }

    list->head = n;
    return n;
}

/// @brief
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

/// @brief Abstraction from using global variable tac.
TACnode *tac_append(OpCode instr, char *a1, char *a2, char *a3)
{
    return tac_list_append(&tac, instr, a1, a2, a3);
}

void insert_before_node(TACnode *ref, TACnode *node)
{
    if (!ref || !node)
        return;

    node->next = ref;
    node->prev = ref->prev;

    if (ref->prev)
        ref->prev->next = node;
    else
        tac.head = node;

    ref->prev = node;
}
