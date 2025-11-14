#include "codegen.h"
#include "common.h"
#include "ast.h"

TAClist tac = { NULL, NULL };

/**
 * @brief GEN_FUNC_TEMPLATE 
int func_name(ASTptr_node) {
    DEBUG_PRINT("Entered function x");

    DEBUG_PRINT("Left function x");
    return 1;
}
*/

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

/* OUTPUT */
    /*
    for (node = head; node != NULL; node = node->next)
        print(node->instr); 
    */
}
