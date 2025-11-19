#include "../../include/ast.h"
#include "../../include/functionTable.h"
#include "../../include/semantic.h"
#include "../../include/common.h"


int main(int argc, char **argv){
    (void)argc;
    printf("Semantic tests\n");

    ASTptr program = malloc(sizeof(ASTnode));

    int state = atoi(argv[1]);
    switch(state){
        case 1:{
            printf("Missing main\n");
            program->type = AST_PROGRAM;
            program->program.funcsCount = 1;
            program->program.funcsCap = 1;
            program->program.funcs = malloc(sizeof(ASTptr) * program->program.funcsCap);

            ASTptr func = malloc(sizeof(ASTnode));
            func->type = AST_FUNC_DEF;
            func->func.name = "nomain";
            func->func.paramCount = 0;
            func->func.paramNames = malloc(sizeof(char*));

            program->program.funcs[0] = func;
            break;
        }
        case 2:{
            printf("Foo function has global variable as argument\n");
            program->type = AST_PROGRAM;
            program->program.funcsCount = 2;
            program->program.funcsCap = 2;
            program->program.funcs = malloc(sizeof(ASTptr) * program->program.funcsCap);

            ASTptr func = malloc(sizeof(ASTnode));
            func->type = AST_FUNC_DEF;
            func->func.name = "main";
            func->func.paramCount = 0;
            func->func.paramNames = malloc(sizeof(char*));

            ASTptr func2 = malloc(sizeof(ASTnode));
            func2->type = AST_FUNC_DEF;
            func2->func.name = "foo";
            func2->func.paramCount = 2;
            func2->func.paramNames = malloc(sizeof(char*));
            func2->func.paramNames[0] = "__a";

            program->program.funcs[0] = func;
            program->program.funcs[1] = func2;
            break;
        }
        case 3:{
            printf("Foo function has two identical parameters\n");
            program->type = AST_PROGRAM;
            program->program.funcsCount = 2;
            program->program.funcsCap = 2;
            program->program.funcs = malloc(sizeof(ASTptr) * program->program.funcsCap);

            ASTptr func = malloc(sizeof(ASTnode));
            func->type = AST_FUNC_DEF;
            func->func.name = "main";
            func->func.paramCount = 0;
            func->func.paramNames = malloc(sizeof(char*));

            ASTptr func2 = malloc(sizeof(ASTnode));
            func2->type = AST_FUNC_DEF;
            func2->func.name = "foo";
            func2->func.paramCount = 2;
            func2->func.paramNames = malloc(sizeof(char*)*2);
            func2->func.paramNames[0] = "a";
            func2->func.paramNames[1] = "a";

            program->program.funcs[0] = func;
            program->program.funcs[1] = func2;
            break;
        }
        default:
            break;
    }
    
    semantic(program);
    return 0;
}