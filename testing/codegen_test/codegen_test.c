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
#include <codegen.h>
#include <ast.h>

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

    generate(test);
    
    return 0;
}
