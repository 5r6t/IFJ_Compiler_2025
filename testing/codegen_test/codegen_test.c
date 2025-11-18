//////////////////////////////////////////////
// filename: lex_test_data.c                //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
// * Jozef Matus (xmatusj00) / karfisk 	    //
// * Jan Hajek (xhajekj00) / Wekk 	        //
//////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <codegen.h>
#include <ast.h>

static ASTptr make_var_decl(const char *name) {
    ASTptr node = calloc(1, sizeof(ASTnode));
    if (!node) {
        return NULL;
    }
    node->type = AST_VAR_DECL;
    node->var_decl.varName = strdup(name);
    return node;
}

static ASTptr make_block(ASTptr *stmts, int count) {
    ASTptr node = calloc(1, sizeof(ASTnode));
    if (!node) {
        return NULL;
    }
    node->type = AST_BLOCK;
    node->block.stmtCount = count;
    node->block.stmtCap = count;
    if (count > 0) {
        node->block.stmt = malloc(sizeof(ASTptr) * count);
        for (int i = 0; i < count; ++i) {
            node->block.stmt[i] = stmts[i];
        }
    }
    return node;
}

static ASTptr make_function(const char *name, char **params, int paramCount, ASTptr body) {
    ASTptr node = calloc(1, sizeof(ASTnode));
    if (!node) {
        return NULL;
    }
    node->type = AST_FUNC_DEF;
    node->func.name = strdup(name);
    node->func.paramCount = paramCount;
    if (paramCount > 0) {
        node->func.paramNames = malloc(sizeof(char *) * paramCount);
        for (int i = 0; i < paramCount; ++i) {
            node->func.paramNames[i] = strdup(params[i]);
        }
    } else {
        node->func.paramNames = NULL;
    }
    node->func.isGetter = false;
    node->func.isSetter = false;
    node->func.body = body;
    return node;
}

static ASTptr make_program(ASTptr *funcs, int count) {
    ASTptr node = calloc(1, sizeof(ASTnode));
    if (!node) {
        return NULL;
    }
    node->type = AST_PROGRAM;
    node->program.funcsCount = count;
    node->program.funcsCap = count;
    if (count > 0) {
        node->program.funcs = malloc(sizeof(ASTptr) * count);
        for (int i = 0; i < count; ++i) {
            node->program.funcs[i] = funcs[i];
        }
    } else {
        node->program.funcs = NULL;
    }
    return node;
}

int main (int argc, char** argv) {
    (void) argc;
    (void) argv;
    ASTptr test = NULL;

    TAClist local_list;
    tac_list_init(&local_list);
    tac_print_list_state("after init", &local_list);

    TACnode *first = tac_list_append(&local_list, DEFVAR, "GF@test", NULL, NULL);
    TACnode *second = tac_list_append(&local_list, MOVE, "GF@test", "int@42", NULL);
    (void)first;
    tac_print_list_state("after append", &local_list);

    TACnode *popped = tac_list_pop_front(&local_list);
    tac_print_node("popped", popped);
    tac_node_free(popped);
    tac_print_list_state("after pop", &local_list);

    tac_list_remove(&local_list, second);
    tac_print_list_state("after remove", &local_list);
    tac_node_free(second);

    tac_list_clear(&local_list);
    tac_print_list_state("after clear", &local_list);

    ASTptr decl = make_var_decl("y");
    ASTptr stmts[] = { decl };
    ASTptr body = make_block(stmts, 1);
    ASTptr funcs_arr[] = { make_function("main", NULL, 0, body) };
    test = make_program(funcs_arr, 1);

    generate(test);
    
    return 0;
}
