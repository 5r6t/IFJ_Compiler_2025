//////////////////////////////////////////////
// filename: codegen.c                      //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//////////////////////////////////////////////

/**
==== TO-DO ====
 * scopedepth ~~ we'll see
* @brief int scopeDepth - indicating depth level for differentiating
    between global, local and temporary variables

=== NOTES ===
- Only GF is initialized on start

- Use NAME WRANGLING e.g.
    var x // DEFVAR LF@x$1
    {
        var x // DEFVAR LF@x$2 where e.g. $ is scope depth
    } // use symtable to determine scope depth

- Function Calls - Stack or Frame versions in the presentation 44/56
- Loops - presentation 46/56
- Global counter for labels - presentation 47/56
- Constants - presentation 48/56
- Type checking - presentation 50/56

// --- Runtime Error Codes ---
#define ERR_RUNTIME_ARG 25  // Runtime: invalid builtin parameter type
#define ERR_RUNTIME_TYPE 26 // Runtime: type mismatch in expr at runtime

*/

#include "codegen.h"
#include "common.h"
#include "ast.h"
#include <stdbool.h>

#define NAME_BUF 256
#define RUNTIME

TAClist tac = {NULL, NULL};
TAClist globdefs = {NULL, NULL};

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

// track whether handled function should return a value
static bool __has_explicit_ret = false;
// track whether main function is being handled
static bool __is_main = false;
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
    tac_list_append(&globdefs, DEFVAR, name, NULL, NULL);
    return;
}

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

/// @brief 
char *lit_nil()
{
    return my_strdup("nil@nil");
}

/// @brief function that determines if a variable is global or local
char *var_gf_or_lf(char *name, int scope_depth)
{
    char buf[NAME_BUF];

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

/// @brief Create string in format: LF@x, where x is a number
static char *new_lf_tmp()
{
    char buf[NAME_BUF];
    static int lf_counter = 0;

    snprintf(buf, sizeof(buf), "LF@%%%d", lf_counter++);
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

/// @brief Create labels in style: <prefix>x, where x is a counter
static char *new_label(const char *prefix)
{
    char buf[128];
    static int label_counter = 0;

    snprintf(buf, sizeof(buf), "%s%d", prefix, label_counter++);
    return my_strdup(buf);
}

///////////////////////////////////
// ---- Built in handling
///////////////////////////////////

// buggy === same labels
static void gen_builtin_substring(ASTptr call, char *result_tmp)
{
    char *s = gen_expr(call->call.args[0]);
    char *i = gen_expr(call->call.args[1]);
    char *j = gen_expr(call->call.args[2]);

    // temps
    char *tmp = new_lf_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);
    char *len = new_lf_tmp();
    tac_append(DEFVAR, len, NULL, NULL);
    char *k = new_lf_tmp();
    tac_append(DEFVAR, k, NULL, NULL);

    char *LBL_i_not_int = new_label("$substr_i_not_int_");
    char *LBL_j_not_int = new_label("$substr_j_not_int_");
    char *LBL_return_nil = new_label("$substr_ret_nil_");
    char *LBL_loop = new_label("$substr_loop_");
    char *LBL_loop_end = new_label("$substr_loop_end_");
    char *LBL_end = new_label("$substr_end_");

    // ----- i must be int -----
    tac_append(ISINT, tmp, i, NULL);
    tac_append(JUMPIFNEQ, LBL_i_not_int, tmp, "bool@true");

    // ----- j must be int -----
    tac_append(ISINT, tmp, j, NULL);
    tac_append(JUMPIFNEQ, LBL_j_not_int, tmp, "bool@true");

    // compute len(s)
    tac_append(STRLEN, len, s, NULL);

    // ----- i < 0 → nil -----
    tac_append(LT, tmp, i, "int@0");
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@true");

    // ----- j < 0 → nil -----
    tac_append(LT, tmp, j, "int@0");
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@true");

    // ----- i > j → nil -----
    tac_append(GT, tmp, i, j);
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@true");

    // ----- i >= len -----
    // compute (i < len)
    tac_append(LT, tmp, i, len);
    // if tmp == true → ok, continue
    tac_append(JUMPIFEQ, LBL_loop, tmp, "bool@true");
    // else → i >= len
    tac_append(JUMP, LBL_return_nil, NULL, NULL);

    // ----- j > len -----
    tac_append(LABEL, LBL_loop, NULL, NULL);
    tac_append(GT, tmp, j, len);
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@true");

    // ----- build substring -----
    // res = ""
    tac_append(MOVE, result_tmp, "string@", NULL);

    // k = i
    tac_append(MOVE, k, i, NULL);

    tac_append(LABEL, LBL_loop, NULL, NULL);

    // if k >= j → end
    // check (k < j)
    tac_append(LT, tmp, k, j);
    tac_append(JUMPIFEQ, LBL_loop_end, tmp, "bool@false");
    // IF tmp is false → !(k < j) → k >= j

    // tmp = s[k]
    tac_append(GETCHAR, tmp, s, k);
    // result_tmp += tmp
    tac_append(CONCAT, result_tmp, result_tmp, tmp);

    // k = k + 1
    tac_append(ADD, k, k, "int@1");

    tac_append(JUMP, LBL_loop, NULL, NULL);

    tac_append(LABEL, LBL_loop_end, NULL, NULL);
    tac_append(JUMP, LBL_end, NULL, NULL);

    // ----- errors -----
    tac_append(LABEL, LBL_i_not_int, NULL, NULL);
    tac_append(EXIT, "int@6", NULL, NULL);

    tac_append(LABEL, LBL_j_not_int, NULL, NULL);
    tac_append(EXIT, "int@6", NULL, NULL);

    // ----- nil -----
    tac_append(LABEL, LBL_return_nil, NULL, NULL);
    tac_append(MOVE, result_tmp, "nil@nil", NULL);

    tac_append(LABEL, LBL_end, NULL, NULL);
}

static void gen_builtin_strcmp(ASTptr call, char *result_tmp)
{
    char *s1 = gen_expr(call->call.args[0]);
    char *s2 = gen_expr(call->call.args[1]);

    // temps
    char *i = new_lf_tmp();
    tac_append(DEFVAR, i, NULL, NULL);
    char *c1 = new_lf_tmp();
    tac_append(DEFVAR, c1, NULL, NULL);
    char *c2 = new_lf_tmp();
    tac_append(DEFVAR, c2, NULL, NULL);
    char *len1 = new_lf_tmp();
    tac_append(DEFVAR, len1, NULL, NULL);
    char *len2 = new_lf_tmp();
    tac_append(DEFVAR, len2, NULL, NULL);
    char *tmp = new_lf_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);

    char *LBL_loop = new_label("$strcmp_loop_");
    char *LBL_loop_end = new_label("$strcmp_end_");
    char *LBL_return = new_label("$strcmp_ret_");
    char *LBL_after_diff = new_label("$strcmp_after_diff_");

    // len1 = length(s1)
    tac_append(STRLEN, len1, s1, NULL);
    // len2 = length(s2)
    tac_append(STRLEN, len2, s2, NULL);

    // i = 0
    tac_append(MOVE, i, "int@0", NULL);

    // loop:
    tac_append(LABEL, LBL_loop, NULL, NULL);

    // if i >= len1 or i >= len2 → go to post-loop
    tac_append(LT, tmp, i, len1);
    tac_append(JUMPIFEQ, LBL_loop_end, tmp, "bool@false"); // !(i < len1)
    tac_append(LT, tmp, i, len2);
    tac_append(JUMPIFEQ, LBL_loop_end, tmp, "bool@false"); // !(i < len2)

    // c1 = ASCII of s1[i]
    tac_append(STRI2INT, c1, s1, i);
    // c2 = ASCII of s2[i]
    tac_append(STRI2INT, c2, s2, i);

    // if c1 == c2 → continue
    tac_append(EQ, tmp, c1, c2);
    tac_append(JUMPIFEQ, LBL_after_diff, tmp, "bool@false"); // if equal → skip diff calc

    // difference → result_tmp = c1 - c2
    tac_append(SUB, result_tmp, c1, c2);
    tac_append(JUMP, LBL_return, NULL, NULL);

    tac_append(LABEL, LBL_after_diff, NULL, NULL);

    // i = i + 1
    tac_append(ADD, i, i, "int@1");

    tac_append(JUMP, LBL_loop, NULL, NULL);

    // loop_end: one string ended or both equal so far
    tac_append(LABEL, LBL_loop_end, NULL, NULL);

    // compare lengths
    tac_append(SUB, result_tmp, len1, len2);

    tac_append(LABEL, LBL_return, NULL, NULL);
}

static void gen_builtin_str(ASTptr call, char *result_tmp)
{
    // argument evaluated first
    char *arg = gen_expr(call->call.args[0]);

    // temp for storing detected type
    char *t = new_lf_tmp();
    tac_append(DEFVAR, t, NULL, NULL);
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

static void gen_builtin_ord(ASTptr call, char *result_tmp)
{
    char *s = gen_expr(call->call.args[0]);
    char *i = gen_expr(call->call.args[1]);

    char *tmp = new_lf_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);
    char *len = new_lf_tmp();
    tac_append(DEFVAR, len, NULL, NULL);

    char *LBL_i_not_int = new_label("$ord_i_not_int_");
    char *LBL_return_nil = new_label("$ord_ret_nil_");
    char *LBL_end = new_label("$ord_end_");

    // -- type check: i must be integer
    tac_append(ISINT, tmp, i, NULL);
    tac_append(JUMPIFNEQ, LBL_i_not_int, tmp, "bool@true");

    // -- compute length of s
    tac_append(STRLEN, len, s, NULL);

    // -- if i < 0 → nil
    tac_append(LT, tmp, i, "int@0");
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@true");

    // -- if i >= len → nil   (i < len must be true)
    tac_append(LT, tmp, i, len);
    tac_append(JUMPIFEQ, LBL_return_nil, tmp, "bool@false");

    // -- valid: STRI2INT result_tmp = s[i]
    tac_append(STRI2INT, result_tmp, s, i);
    tac_append(JUMP, LBL_end, NULL, NULL);

    // -- not int → error 6
    tac_append(LABEL, LBL_i_not_int, NULL, NULL);
    tac_append(EXIT, "int@6", NULL, NULL);

    // -- nil return
    tac_append(LABEL, LBL_return_nil, NULL, NULL);
    tac_append(MOVE, result_tmp, "nil@nil", NULL);

    tac_append(LABEL, LBL_end, NULL, NULL);
}

static void gen_builtin_chr(ASTptr call, char *result_tmp)
{
    char *i = gen_expr(call->call.args[0]);

    char *tmp = new_lf_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);

    char *LBL_i_not_int = new_label("$chr_i_not_int_");
    char *LBL_err_range = new_label("$chr_err_range_");
    char *LBL_end = new_label("$chr_end_");

    // -- type check: must be integer
    tac_append(ISINT, tmp, i, NULL);
    tac_append(JUMPIFNEQ, LBL_i_not_int, tmp, "bool@true");

    // -- check i < 0 → error 6
    tac_append(LT, tmp, i, "int@0");
    tac_append(JUMPIFEQ, LBL_err_range, tmp, "bool@true");

    // -- check i > 255 → error 6
    tac_append(GT, tmp, i, "int@255");
    tac_append(JUMPIFEQ, LBL_err_range, tmp, "bool@true");

    // -- valid: INT2CHAR
    tac_append(INT2CHAR, result_tmp, i, NULL);
    tac_append(JUMP, LBL_end, NULL, NULL);

    // -- not int → error 26
    // -- out of range → error 26
    tac_append(LABEL, LBL_i_not_int, NULL, NULL);
    tac_append(LABEL, LBL_err_range, NULL, NULL);
    EMIT_ARG_EXIT();

    // END
    tac_append(LABEL, LBL_end, NULL, NULL);
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
        if (strcmp(arr_builtin_names[i], func_name) == 0) {
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
        tac_append(DEFVAR, tmp, NULL, NULL);

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
        gen_builtin_substring(call, result_tmp);
        return;
    }
    if (strcmp(name, "Ifj.strcmp") == 0)
    {
        gen_builtin_strcmp(call, result_tmp);
        return;
    }
    if (strcmp(name, "Ifj.ord") == 0)
    {
        gen_builtin_ord(call, result_tmp);
        return;
    }

    if (strcmp(name, "Ifj.chr") == 0)
    {
        gen_builtin_chr(call, result_tmp);
        return;
    }
}
///////////////////////////////////
// ---- AST Nodes Codegen ----
///////////////////////////////////

/// @brief
void gen_block(ASTptr node)
{
    for (int i = 0; i < node->block.stmtCount; i++)
    {
        gen_stmt(node->block.stmt[i]);
    }
}

/// @brief function that appends instructions for a definition of a function to a list
void gen_func_def(ASTptr node)
{
    // check if functions is main -- stored in global variable
    __is_main = (strcmp(node->func.name, "main") == 0) &&
                (node->func.paramCount == 0);

    // label is either $$main or $func_name
    char *label = __is_main ? my_strdup("$$main") : fnc_label(node->func.name);
    tac_append(LABEL, label, NULL, NULL);
    char *retval = "LF@%retval1";

    if (__is_main == true)
    {
        tac_append(CREATEFRAME, NULL, NULL, NULL);
        tac_append(PUSHFRAME, NULL, NULL, NULL);
        tac_append(DEFVAR, retval, NULL, NULL);
    }
    else
    {
        tac_append(PUSHFRAME, NULL, NULL, NULL);
        tac_append(MOVE, retval, "nil@nil", NULL);
    }

    if (__is_main == false)
    {
        // handle parameters
        for (int i = 0; i < node->func.paramCount; i++)
        {
            // ASTptr argNode = node->call.args[i]; forgot what i wanted
            char *local_param = var_lf(node->func.paramNames[i]);
            tac_append(DEFVAR, local_param, NULL, NULL);

            char srcbuf[NAME_BUF];
            snprintf(srcbuf, sizeof(srcbuf), "LF@%%%d", i + 1); // start with 1

            tac_append(MOVE, local_param, my_strdup(srcbuf), NULL);
        }
    }

    // Generate function body
    __has_explicit_ret = false;
    gen_block(node->func.body);

    // function epilogue
    if (!__has_explicit_ret)
    {
        tac_append(POPFRAME, NULL, NULL, NULL);
        if (!__is_main) // do not return on empty stack
        {
            tac_append(RETURN, NULL, NULL, NULL);
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
        char *tmp = new_lf_tmp();
        tac_append(DEFVAR, tmp, NULL, NULL);
        gen_builtin_call(node, tmp);
        return;
    }

    // --- USER-DEFINED FUNCTION CALL ---
    tac_append(CREATEFRAME, NULL, NULL, NULL);

    for (int i = 0; i < node->call.argCount; i++)
    {
        char buf[NAME_BUF];
        snprintf(buf, sizeof(buf), "TF@%%%d", i + 1);
        tac_append(DEFVAR, buf, NULL, NULL);

        char *arg = gen_expr(node->call.args[i]);
        tac_append(MOVE, buf, arg, NULL);
    }

    char *func_label = fnc_label(fname);
    tac_append(CALL, func_label, NULL, NULL);
}

/// @brief declaration of a variable
void gen_var_decl(ASTptr node, int scope_depth)
{
    char *var = var_gf_or_lf(node->var_decl.varName, scope_depth);
    tac_append(DEFVAR, var, NULL, NULL);
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
        target = var_lf(node->assign_stmt.targetName);
    }

    char *expr = gen_expr(node->assign_stmt.expr);
    tac_append(MOVE, target, expr, NULL);
    return;
}

/// @brief
void gen_while_stmt(ASTptr node)
{
    (void)node;
    return;
}

void gen_if_stmt(ASTptr node)
{
    // 1. Condition
    char *cond = gen_expr(node->ifstmt.cond);

    // 2. Labels names
    char *lbl_else = new_label("$if_else_");
    char *lbl_end = new_label("$if_end_");

    // 3. Shall Condition be false --> exec ELSE block
    tac_append(JUMPIFEQ, lbl_else, cond, "bool@false");

    // 4. IF block
    gen_block(node->ifstmt.then);

    // 5. JUMP to ELSE block
    tac_append(JUMP, lbl_end, NULL, NULL);

    // 6. ELSE label
    tac_append(LABEL, lbl_else, NULL, NULL);

    // 7. ELSE block (if exists)
    if (node->ifstmt.elsestmt != NULL)
        gen_block(node->ifstmt.elsestmt);

    // 8. final end label
    tac_append(LABEL, lbl_end, NULL, NULL);
}

void gen_return_stmt(ASTptr node, int scopeDepth)
{
    (void)scopeDepth;

    __has_explicit_ret = true;

    char *expr = gen_expr(node->return_stmt.expr);
    tac_append(MOVE, "LF@%retval1", expr, NULL);

    if (__is_main)
    {
        return;
    }

    tac_append(POPFRAME, NULL, NULL, NULL);
    tac_append(RETURN, NULL, NULL, NULL);
    return;
}

///////////////////////////////////
// ---- BINARY OPERATORS  ----
///////////////////////////////////
// string + string or num + num, but not num + string/null --> ERR 26
char *gen_binop_add(char *res, char *l, char *r)
{
    char *t_l = new_lf_tmp();
    char *t_r = new_lf_tmp();
    tac_append(DEFVAR, t_l, NULL, NULL);
    tac_append(DEFVAR, t_r, NULL, NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_str_ok = new_label("$add_str_");
    char *L_num_ok = new_label("$add_num_");
    char *L_error = new_label("$add_err_");
    char *L_end = new_label("$add_end_");

    // string + string ?
    tac_append(JUMPIFEQ, L_str_ok, t_l, "string@string");
    tac_append(JUMPIFEQ, L_error, t_r, "string@string"); // if only right is string → error
    //   (if left wasn't string, above would not trigger unless mismatch)

    // numeric + numeric ?
    tac_append(JUMPIFEQ, L_num_ok, t_l, "string@int");
    tac_append(JUMPIFEQ, L_num_ok, t_l, "string@float");

    // error if types mismatched
    tac_append(JUMP, L_error, NULL, NULL);

    // ----- string concat -----
    tac_append(LABEL, L_str_ok, NULL, NULL);
    tac_append(JUMPIFNEQ, L_error, t_r, "string@string");
    tac_append(CONCAT, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    // ----- numeric add -----
    tac_append(LABEL, L_num_ok, NULL, NULL);
    tac_append(JUMPIFNEQ, L_error, t_r, "string@int");
    // OR float automatically works since ADD handles both
    tac_append(ADD, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    // ----- error -----
    tac_append(LABEL, L_error, NULL, NULL);
    EMIT_TYPE_EXIT();

    // ----- end -----
    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}
char *gen_binop_sub(char *res, char *l, char *r)
{
    char *t_l = new_lf_tmp();
    char *t_r = new_lf_tmp();
    tac_append(DEFVAR, t_l, NULL, NULL);
    tac_append(DEFVAR, t_r, NULL, NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_ok = new_label("$sub_ok_");
    char *L_err = new_label("$sub_err_");
    char *L_end = new_label("$sub_end_");

    tac_append(JUMPIFEQ, L_ok, t_l, "string@int");
    tac_append(JUMPIFEQ, L_ok, t_l, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(LABEL, L_ok, NULL, NULL);
    tac_append(JUMPIFEQ, L_end, t_r, "string@int");
    tac_append(JUMPIFEQ, L_end, t_r, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(SUB, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    tac_append(LABEL, L_err, NULL, NULL);
    tac_append(EXIT, "int@26", NULL, NULL);

    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}
// BAD
char *gen_binop_mul(char *res, char *l, char *r)
{
    char *t_l = new_lf_tmp();
    char *t_r = new_lf_tmp();
    tac_append(DEFVAR, t_l, NULL, NULL);
    tac_append(DEFVAR, t_r, NULL, NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_ok = new_label("$mul_ok_");
    char *L_err = new_label("$mul_err_");
    char *L_end = new_label("$mul_end_");

    tac_append(JUMPIFEQ, L_ok, t_l, "string@int");
    tac_append(JUMPIFEQ, L_ok, t_l, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(LABEL, L_ok, NULL, NULL);
    tac_append(JUMPIFEQ, L_end, t_r, "string@int");
    tac_append(JUMPIFEQ, L_end, t_r, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(MUL, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    tac_append(LABEL, L_err, NULL, NULL);
    EMIT_TYPE_EXIT();

    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}

// todo
char *gen_binop_div(char *res, char *l, char *r)
{
    char *t_l = new_lf_tmp();
    char *t_r = new_lf_tmp();
    char *t_zero = new_lf_tmp();
    tac_append(DEFVAR, t_l, NULL, NULL);
    tac_append(DEFVAR, t_r, NULL, NULL);
    tac_append(DEFVAR, t_zero, NULL, NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_ok = new_label("$div_ok_");
    char *L_z = new_label("$div_zero_");
    char *L_err = new_label("$div_err_");
    char *L_end = new_label("$div_end_");

    tac_append(JUMPIFEQ, L_ok, t_l, "string@int");
    tac_append(JUMPIFEQ, L_ok, t_l, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(LABEL, L_ok, NULL, NULL);
    tac_append(JUMPIFEQ, L_end, t_r, "string@int");
    tac_append(JUMPIFEQ, L_end, t_r, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    // check zero
    tac_append(EQ, t_zero, r, "float@0x0p+0");
    tac_append(JUMPIFEQ, L_z, t_zero, "bool@true");

    tac_append(DIV, res, l, r);
    tac_append(JUMP, L_end, NULL, NULL);

    tac_append(LABEL, L_z, NULL, NULL);
    tac_append(EXIT, "int@9", NULL, NULL);

    tac_append(LABEL, L_err, NULL, NULL);
    EMIT_TYPE_EXIT();

    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}

// todo
char *gen_binop_rel(char *res, char *l, char *r, BinOpType rel)
{
    char *t_l = new_lf_tmp();
    char *t_r = new_lf_tmp();
    tac_append(DEFVAR, t_l, NULL, NULL);
    tac_append(DEFVAR, t_r, NULL, NULL);

    tac_append(TYPE, t_l, l, NULL);
    tac_append(TYPE, t_r, r, NULL);

    char *L_types_eq = new_label("$rel_tyeq_");
    char *L_null_null = new_label("$rel_nullnull_");
    char *L_num = new_label("$rel_numok_");
    char *L_do = new_label("$rel_do_");
    char *L_false = new_label("$rel_false_");
    char *L_end = new_label("$rel_end_");
    char *L_err = new_label("$rel_err_");

    /* ================================
         HANDLE == and != first
       ================================ */
    if (rel == BINOP_EQ || rel == BINOP_NEQ)
    {

        // if types equal → go compare
        tac_append(EQ, res, t_l, t_r);
        tac_append(JUMPIFEQ, L_types_eq, res, "bool@true");

        // types differ → null == null special case?
        // if both null types?
        tac_append(JUMPIFEQ, L_null_null, t_l, "string@nil");
        tac_append(JUMP, L_false, NULL, NULL);

        tac_append(LABEL, L_null_null, NULL, NULL);
        tac_append(JUMPIFEQ, L_false, t_r, "string@nil");

        // both null → true
        tac_append(MOVE, res, "bool@true", NULL);
        tac_append(JUMP, L_end, NULL, NULL);

        // types same
        tac_append(LABEL, L_types_eq, NULL, NULL);

        // if null == null already handled; if both null return true
        tac_append(JUMPIFEQ, L_null_null, t_l, "string@nil");

        // Compare using EQ
        tac_append(EQ, res, l, r);

        if (rel == BINOP_NEQ)
        {
            // invert boolean
            tac_append(JUMPIFEQ, L_false, res, "bool@true");
            tac_append(MOVE, res, "bool@true", NULL); // was false → true
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

    /* ================================
         HANDLE <, >, <=, >= (NUM ONLY)
       ================================ */

    // left must be int/float
    tac_append(JUMPIFEQ, L_num, t_l, "string@int");
    tac_append(JUMPIFEQ, L_num, t_l, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(LABEL, L_num, NULL, NULL);
    // right must be int/float
    tac_append(JUMPIFEQ, L_do, t_r, "string@int");
    tac_append(JUMPIFEQ, L_do, t_r, "string@float");
    tac_append(JUMP, L_err, NULL, NULL);

    tac_append(LABEL, L_do, NULL, NULL);

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
        // l <= r  ⇢  !(l > r)
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
        // l >= r ⇢ !(l < r)
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
    tac_append(LABEL, L_err, NULL, NULL);
    tac_append(EXIT, "int@26", NULL, NULL);

    tac_append(LABEL, L_end, NULL, NULL);
    return res;
}

// todo
char *gen_binop_eq(char *res, char *l, char *r, BinOpType eq)
{
    if (eq == BINOP_EQ)
        tac_append(EQ, res, l, r);
    return res;
}
char *gen_binop_is(char *res, char *l, char *r, TypeName targetType)
{
    char *type_tmp = new_lf_tmp();
    if (targetType)
    {
    }
    tac_append(DEFVAR, type_tmp, l, "FUCKING WRONG");
    tac_append(DEFVAR, type_tmp, r, "FUCKING WRONG");
    tac_append(DEFVAR, type_tmp, res, "FUCKING WRONG");
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

    char *res = new_lf_tmp();
    tac_append(DEFVAR, res, NULL, NULL);
    // char*possible_lit_left=gen_literal()
    ;
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
        return gen_binop_is(res, left, right, node->binop.resultType);

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
    // global
    if (node->identifier.idType == ID_GLOBAL)
    {
        return var_gf(node->identifier.name); // GF@__blabla
    }
    // local
    if (node->identifier.idType == ID_LOCAL)
    {
        return var_lf(node->identifier.name); // LF@bla
    }

    // getter
    if (node->identifier.idType == ID_GETTER)
    {
        char *tmp = new_lf_tmp();
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
char *gen_literal(ASTptr node)
{
    char *r;
    switch (node->literal.liType)
    {
    case LIT_NULL:
        r = lit_nil();
        return r;

        /* Extension not implemented
        case LIT_BOOL:
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
        gen_if_stmt(node);
        break;
    case AST_WHILE_STMT:
        gen_while_stmt(node);
        break;
    case AST_FUNC_CALL:
        gen_func_call(node);
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
    char *tmp = new_lf_tmp();
    tac_append(DEFVAR, tmp, NULL, NULL);

    // Prepare call frame
    if (is_builtin(fname))
    {
        gen_builtin_call(node, tmp);
        return tmp;
    }

    // TF
    tac_append(CREATEFRAME, NULL, NULL, NULL);
    // For each argument TF@1 .. TF@n
    for (int i = 0; i < node->call.argCount; i++)
    {
        char tfname[NAME_BUF];
        snprintf(tfname, sizeof(tfname), "TF@%%%d", i + 1);
        tac_append(DEFVAR, my_strdup(tfname), NULL, NULL);

        char *arg = gen_expr(node->call.args[i]);
        tac_append(MOVE, my_strdup(tfname), arg, NULL);
    }
    // return slot
    char *retval = "TF@%retval1";
    tac_append(DEFVAR, retval, NULL, NULL);
    // call
    char *func_lab = fnc_label(fname);
    tac_append(CALL, func_lab, NULL, NULL);
    // copy into local tmp
    tac_append(MOVE, tmp, retval, NULL);
    return tmp;
}

///////////////////////////////////
// ---- Traversal and Output
///////////////////////////////////

/// @brief appends prologue of a program to a list, iterates over gen_func_def
void gen_program(ASTptr node)
{
    // header
    tac_append(JUMP, "$$main", NULL, NULL);

    for (int i = 0; i < node->program.funcsCount; i++)
    {
        gen_func_def(node->program.funcs[i]);
    }
    return;
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
        fprintf(stderr, "gen_expr: unsupported AST node -> type %d\n", node->type);
    }
    return NULL; // reach only on errors
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
    fprintf(stderr, "%s", new_tf_tmp()); // just to not cause problems

    tac_list_init(&globdefs);
    tac_list_init(&tac);
    gen_program(tree);

    /* OUTPUT -- no optimalizations */
    {
        // print header
        printf(".IFJcode25\n");
        // todo add func to list to replace head (keeping original as next)
        // declare globals
        print_tac(globdefs);
        // print the rest of the program
        print_tac(tac);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
// ---- TAC Related ----
///////////////////////////////////

/// @brief prints the entire list to standard output
void print_tac(TAClist list)
{
    for (const TACnode *curr = list.head; curr; curr = curr->next)
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
TACnode *tac_append(OpCode instr, char *a1, char *a2, char *a3)
{
    return tac_list_append(&tac, instr, a1, a2, a3);
}
