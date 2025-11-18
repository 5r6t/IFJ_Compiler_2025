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
TAClist tac = { NULL, NULL };

/** TO-DO
 * scopedepth ~~ we'll see
 * 
 */

// used for printing NAMES of operands, see codegen.h
static const char *tac_opcode_name(OpCode op) {
    switch (op) {
#define OPCODE_CASE(name) case name: return #name;
        TAC_OPCODE_LIST(OPCODE_CASE)
#undef OPCODE_CASE
        default:
            return "UNKNOWN";
    }
}

void tac_print_list_state(const char *label, const TAClist *list) {
    const char *tag = label ? label : "TAC list";
    size_t count = 0;
    const TACnode *curr = (list ? list->head : NULL);
    // count number of nodes
    while (curr) {
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

void tac_print_node(const char *label, const TACnode *node) {
    const char *tag = label ? label : "TAC node";
    printf("\n-- TAC NODE :: %s --\n", tag);
    if (node == NULL) {
        printf("  (null)\n");
        return;
    }

    // printf("  addr  : %p\n", (void *)node);
    printf("\tinstr : %s (%d)\n", tac_opcode_name(node->instr), node->instr);
    printf("\targs  : %s | %s | %s\n",
            node->a1 ? node->a1 : "NULL",
            node->a2 ? node->a2 : "NULL",
            node->a3 ? node->a3 : "NULL");
}



void tac_list_init(TAClist *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->tail = NULL;
}

bool tac_list_is_empty(const TAClist *list) {
    return (list == NULL) || (list->head == NULL);
}

TACnode *tac_list_append(TAClist *list, OpCode instr, const char *a1, const char *a2, const char *a3) {
    if (list == NULL) {
        return NULL;
    }

    TACnode *n = malloc(sizeof(TACnode));
    if (n == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    n->instr = instr;
    n->a1 = a1 ? my_strdup(a1) : NULL;
    n->a2 = a2 ? my_strdup(a2) : NULL;
    n->a3 = a3 ? my_strdup(a3) : NULL;
    n->next = NULL;
    n->prev = list->tail;

    if (list->tail) {
        list->tail->next = n;
    } else {
        list->head = n;
    }
    list->tail = n;

    return n;
}

TACnode *tac_list_pop_front(TAClist *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    TACnode *node = list->head;
    list->head = node->next;
    if (list->head) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }
    node->next = NULL;
    node->prev = NULL;

    return node;
}

void tac_list_remove(TAClist *list, TACnode *node) {
    if (list == NULL || node == NULL) {
        return;
    }

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    node->next = NULL;
    node->prev = NULL;
}

void tac_node_free(TACnode *node) {
    if (node == NULL) {
        return;
    }

    free(node->a1);
    free(node->a2);
    free(node->a3);
    free(node);
}

void tac_list_clear(TAClist *list) {
    if (list == NULL) {
        return;
    }

    TACnode *curr = list->head;
    while (curr) {
        TACnode *next = curr->next;
        tac_node_free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
}

/**
 * @brief GEN_FUNC_TEMPLATE 
int func_name(ASTptr_node) {
    DEBUG_PRINT("Entered function x");

    DEBUG_PRINT("Left function x");
    return 1;
}
*/

/**
 * @brief function that appends the next TAC intruction to double-linked list,
 * abstracting from using global TAClist structure
 */
TACnode* tac_append(OpCode instr, char *a1, char *a2, char *a3) {
    return tac_list_append(&tac, instr, a1, a2, a3);
}

/**
 * @brief Creates string in "LABEL $x" format, x is a string
 */
char *new_label(char* name) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LABEL $%s", name);
    return my_strdup(buf);
}

char* var_lf(const char *name) {
    char buf[NAME_BUF];
    snprintf(buf, sizeof(buf), "LF@%s", name);
    return my_strdup(buf);
}

/// @brief create global variable
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
    snprintf(final, sizeof(final), "string@%s", buf);
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

void gen_program(ASTptr node)
{
    printf(".IFJcode25\n");
    tac_append(JUMP, "$$main", NULL, NULL);
}

void gen_func_def(ASTptr node) 
{
    bool is_main = false;
    char *label;
    if (strcmp(node->func.name, "main") == 0) {
        label = "LABEL $$main";
        is_main = true;
    } else {
        label = new_label(node->func.name);
    }
    tac_append(LABEL, label, NULL, NULL);
    if (is_main == true) {
        tac_append(CREATEFRAME, NULL, NULL, NULL);
    }
    tac_append(PUSHFRAME, NULL, NULL, NULL);
    
    // handle parameters
    for (int i = 0; i < node->func.paramCount; i++) {
        char* local_param = var_lf(node->func.paramNames[i]);
        tac_append(DEFVAR, local_param, NULL, NULL);
        char* local_var = var_lf(node->call.args[i]);
        tac_append(MOVE, local_param, local_var, NULL);
    }

    // handle body - TODOOOO


    if (is_main == false) {
        tac_append(POPFRAME, NULL, NULL, NULL);
        tac_append(RETURN, NULL, NULL, NULL);
    }  
}
void gen_func_call(ASTptr node) {

}
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


/// @brief prints the entire list to standard output 
void print_tac(void) {
    for (const TACnode *curr = tac.head; curr; curr = curr->next) {
        printf("%s", tac_opcode_name(curr->instr));
        if (curr->a1) printf(" %s", curr->a1);
        if (curr->a2) printf(" %s", curr->a2);
        if (curr->a3) printf(" %s", curr->a3);
        putchar('\n');
    }
}



bool handle_node (ASTptr node) {
    switch(node->type) {
    case AST_PROGRAM:       gen_program(node);  break; // return int function(); // int being fail/success
    case AST_FUNC_DEF:      gen_func_def(node); break;
    case AST_FUNC_CALL:     break;
    case AST_BLOCK:         break;
    case AST_IF_STMT:       break;
    case AST_RETURN_STMT:   break;
    // assumes global for now
    case AST_VAR_DECL:      gen_var_decl(node, 0); break;
    case AST_ASSIGN_STMT:   break;
    case AST_WHILE_STMT:    break;
    case AST_IDENTIFIER:    break;
    case AST_LITERAL:       break;
    case AST_BINOP :        break;
    default:
        /* invalid node? */
        DEBUG_PRINT("Invalid node encountered");
        break;
    }
    return true;
}

// Entry point
void generate(ASTptr tree) 
{
    //if (!tree) exit(ERR_INTERNAL);
    gen_program(tree);

    /* OUTPUT -- no optimalizations */
    print_tac();
}
