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
char* var_gf(const char *name);
char* var_tf(const char *name);



char* lit_int(int x);
char* lit_lit_float(double x);
char* lit_string(const char *x);
char* lit_nil();


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