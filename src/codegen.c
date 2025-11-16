#include "codegen.h"
#include "common.h"
#include "ast.h"

#define NAME_BUF 32
TAClist tac = { NULL, NULL };

/**
 * @brief GEN_FUNC_TEMPLATE 
int func_name(ASTptr_node) {
    DEBUG_PRINT("Entered function x");

    DEBUG_PRINT("Left function x");
    return 1;
}
*/

/**
 * @brief function that appends the next TAC intruction to double-linked list
 */
TACnode* tac_append(OpCode instr, char *a1, char *a2, char *a3) {
    TACnode *n = malloc(sizeof(TACnode));
    if (n == NULL) {
        DEBUG_PRINT("Memory allocation error\n");
        return NULL;
    }
    n->instr = instr;
    n->a1 = a1 ? my_strdup(a1) : NULL;
    n->a2 = a2 ? my_strdup(a2) : NULL;
    n->a3 = a3 ? my_strdup(a3) : NULL;
    n->next = NULL;
    n->prev = tac.tail;

    if (tac.tail) {
        tac.tail->next = n;
    }
    else {
        tac.head = n;
    }
    tac.tail = n;
    return n;
}

/**
 * @brief Creates string in "LABEL $c" format, c is a number
 */
char *new_label() {
    static int cnt = 0; // ensure new label number
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LABEL $%d", cnt++);
    return my_strdup(buf);
}

char* var_lf(const char *name) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LF@%s", name);
    return my_strdup(buf);
}
/// @brief create global variable

/// @param name 
/// @return 
char* var_gf(const char *name) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "GF@%s", name);
    return my_strdup(buf);
}
char* var_tf(const char *name) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "TF@%s", name);
    return my_strdup(buf);
}


// ---- Data types conversions ----
char* lit_int(long long x) {
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "int@%lld", x);
    return my_strdup(buf);
}
char* lit_bool(bool x) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "bool@%s", x ? "true" : "false");
    return my_strdup(buf);
}
char* lit_lit_float(double x) {
    char buf[NAME_BUF * 2];
    snprintf(buf, sizeof(buf), "float@%a", x);
    return my_strdup(buf);
}
char* lit_string(const char *x) {
    char buf[4096];
    char *out = buf;

    while (*x) {
        unsigned char c = (unsigned char)*x++;

        // catch escape characters 0-32, 35 (#) and 92 (\)
        if (c <= 32 || c == '#' || c == '\\') {
            out += sprintf(out, "\\%03u", c);
        }
        else {
            *out++ = c;
        }
    }
    *out = '\0';

    char final[4200];
    snprintf(final, sizeof(final), "string2%s", buf);
    return my_strdup(final);
}
char* lit_nil() {
    return my_strdup("nil@nil");
}
/**
 * @brief function that determines if a variable is global or local
 */
char* var_gf_or_lf(char *name, int scope_depth) {
    char buf[64];

    if (scope_depth == 0) {
        snprintf(buf, sizeof(buf), "GF@%s", name);
    }
    else {
        snprintf(buf, sizeof(buf), "LF@%s", name);
    }
    return my_strdup(buf);
}

/**
* @brief int scopeDepth - indicating depth level for differentiating
    between global, local and temporary variables
*/
void gen_program(ASTptr node, int scopeDepth);
void gen_func_def(ASTptr node);
void gen_func_call(ASTptr node);
void gen_block(ASTptr node);
void gen_if_stmt(ASTptr node, int scopeDepth);
void gen_return_stmt(ASTptr node, int scopeDepth);
// declaration of a variable
void gen_var_decl(ASTptr node, int scope_depth) {
    char *var = var_gf_or_lf(node->var_decl.varName, scope_depth);
    tac_append(DEFVAR, var, NULL, NULL);
}
void gen_assign_stmt(ASTptr node);
void gen_while_stmt(ASTptr node, int scopeDepth);
void gen_identifier(ASTptr node);
void gen_literal(ASTptr node);
void gen_binop(ASTptr node) {
    switch(node->binop.opType) {
        case BINOP_ADD:     break;
        case BINOP_SUB:     break;
        case BINOP_MUL:     break;
        case BINOP_DIV:     break;
        case BINOP_LT:      break;
        case BINOP_GT:      break;
        case BINOP_EQ:      break;
        case BINOP_NEQ:     break;
        case BINOP_LTE:     break;
        case BINOP_GTE:     break;
        case BINOP_AND:     break;
        case BINOP_OR:      break;
        case BINOP_IS:      break;
        default:            break;
    }
}


TACnode* get_tac_head ();

/// @brief prints the entire list to standard output 
void print_tac() {
    DEBUG_PRINT("NOT DONE");
}


void handle_node (ASTptr node) {
    switch(node->type) {
    case AST_PROGRAM:       break;
    case AST_FUNC_DEF:      break;
    case AST_FUNC_CALL:     break;
    case AST_BLOCK:         break;
    case AST_IF_STMT:       break;
    case AST_RETURN_STMT:   break;
    case AST_VAR_DECL:      break;
    case AST_ASSIGN_STMT:   break;
    case AST_WHILE_STMT:    break;
    case AST_IDENTIFIER:    break;
    case AST_LITERAL:       break;
    case AST_BINOP :        break;
    default:
        /* invalid node? */
        break;
    }
}

void generate(ASTptr tree) {
    // create header .IFJcode25
    (void) tree;
/* OUTPUT */
    print_tac();
    /*
    for (node = head; node != NULL; node = node->next)
        print(node->instr); 
    */
}