//////////////////////////////////////////////
// filename: lex_test_data.c                //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
//////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <codegen.h>
#include <ast.h>


static ASTptr make_var_decl(const char *name) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_VAR_DECL;
    n->var_decl.varName = strdup(name);
    return n;
}

static ASTptr make_literal_string(const char *s) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_LITERAL;
    n->literal.liType = LIT_STRING;
    n->literal.str = strdup(s);
    return n;
}

static ASTptr make_identifier(const char *name, IdType type) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_IDENTIFIER;
    n->identifier.name = strdup(name);
    n->identifier.idType = type;
    return n;
}

static ASTptr make_assign(const char *name, ASTptr expr) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_ASSIGN_STMT;
    n->assign_stmt.targetName = strdup(name);
    n->assign_stmt.expr = expr;
    n->assign_stmt.asType = TARGET_LOCAL;
    return n;
}

static ASTptr make_binop_add(ASTptr left, ASTptr right) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_BINOP;
    n->binop.opType = BINOP_ADD;
    n->binop.left = left;
    n->binop.right = right;
    return n;
}

static ASTptr make_func_call(const char *name, ASTptr *args, int count) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_FUNC_CALL;

    n->call.funcName = strdup(name);
    n->call.argCount = count;
    n->call.args = malloc(sizeof(ASTptr) * count);

    for (int i = 0; i < count; ++i)
        n->call.args[i] = args[i];

    n->call.funcType = FUNC_INBUILD; // optional
    return n;
}

static ASTptr make_block(ASTptr *stmts, int count) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_BLOCK;
    n->block.stmtCount = count;
    n->block.stmtCap = count;
    n->block.stmt = malloc(sizeof(ASTptr) * count);
    for (int i = 0; i < count; ++i)
        n->block.stmt[i] = stmts[i];
    return n;
}

static ASTptr make_function(const char *name, ASTptr body) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_FUNC_DEF;
    n->func.name = strdup(name);
    n->func.paramCount = 0;
    n->func.paramNames = NULL;
    n->func.isGetter = false;
    n->func.isSetter = false;
    n->func.body = body;
    return n;
}

static ASTptr make_program(ASTptr *funcs, int count) {
    ASTptr n = calloc(1, sizeof(ASTnode));
    n->type = AST_PROGRAM;
    n->program.funcsCount = count;
    n->program.funcsCap = count;
    n->program.funcs = malloc(sizeof(ASTptr) * count);
    for (int i = 0; i < count; ++i)
        n->program.funcs[i] = funcs[i];
    return n;
}




int main () {
    ASTptr test = NULL;

    { // TACLIST TEST
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
    }

    { // MOCK AST 
            
         // var s1
    ASTptr decl_s1 = make_var_decl("s1");

    // s1 = "Hello"
    ASTptr asg_s1 = make_assign("s1", make_literal_string("Hello"));

    // var s2
    ASTptr decl_s2 = make_var_decl("s2");

    // s2 = " World"
    ASTptr asg_s2 = make_assign("s2", make_literal_string(" World"));

    // var s3
    ASTptr decl_s3 = make_var_decl("s3");

    // s3 = s1 + s2
    ASTptr add_s1_s2 = make_binop_add(
        make_identifier("s1", ID_LOCAL),
        make_identifier("s2", ID_LOCAL)
    );
    ASTptr asg_s3 = make_assign("s3", add_s1_s2);

    // __d = Ifj.write(s3)
    ASTptr call_args[1] = {
        make_identifier("s3", ID_LOCAL)
    };
    ASTptr call_write = make_func_call("Ifj.write", call_args, 1);
    ASTptr asg_d = make_assign("__d", call_write);

    // main block stmts
    ASTptr stmts[] = {
        decl_s1, asg_s1,
        decl_s2, asg_s2,
        decl_s3, asg_s3,
        asg_d
    };
    ASTptr body = make_block(stmts, 7);

    // main function
    ASTptr mainf = make_function("main", body);

    // full program
    ASTptr funcs_arr[] = { mainf };
    ASTptr program = make_program(funcs_arr, 1);

        fprintf(stderr, "_________________AST PRINTED___________________\n");
        astPrint(program,0);    

        fprintf(stderr, "__________________CODEGEN RESULT______________________\n");
        generate(program);
    }

    return 0;
}
