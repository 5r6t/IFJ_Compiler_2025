#include "../../include/ast.h"
#include "../../include/functionTable.h"
#include "../../include/semantic.h"
#include "../../include/common.h"

ASTptr makeProgram();
ASTptr makeFunction(char *name, int paramCount);
ASTptr makeBlock();
ASTptr makeVarDecl(char *varName);
ASTptr makeAssignStmt(char *targetName, ASTptr expr);
ASTptr makeIdentifier(char *name, IdType idType);
ASTptr makeLiteralString(char *value);
ASTptr makeLiteralNumber(double value);
ASTptr makeBinOp(BinOpType opType, ASTptr left, ASTptr right);

int main(int argc, char **argv){
    (void)argc;
    printf("Semantic tests\n");
    int state = atoi(argv[1]);

    ASTptr program = makeProgram();
    ASTptr mainFunc = NULL;

    if(state != 1){
        mainFunc = makeFunction("main", 0);
        program->program.funcs[program->program.funcsCount++] = mainFunc;
    }
    
    switch(state){
        case 1:{
            // missing main
            break;
        }
        case 2:{
            // function has global variable as argument
            ASTptr foo = makeFunction("foo", 1);
            foo->func.paramNames[0] = "__a";
            program->program.funcs[program->program.funcsCount++] = foo;
            break;
        }
        case 3:{
            // function has redefined argument
            ASTptr foo = makeFunction("foo", 2);
            foo->func.paramNames[0] = "a";
            foo->func.paramNames[1] = "a";
            program->program.funcs[program->program.funcsCount++] = foo;
            break;
        }
        case 4:{
            // unknown variable in assignment
            ASTptr id = makeIdentifier("a", ID_LOCAL);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = makeAssignStmt("x", id);
            break;
        }
        case 5:{
            // declaring variable as global
            ASTptr v = makeVarDecl("__x");
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v;
            break;
        }
        case 6: {
            // variable redeclaration
            ASTptr v1 = makeVarDecl("a");
            ASTptr v2 = makeVarDecl("a");

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v1;
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v2;
            break;
        }
        case 7: {
            // valid variable declaration
            ASTptr v = makeVarDecl("a");
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v;
            break;
        }
        case 8: {
            // valid assignment to global variable
            ASTptr expr = makeIdentifier("__y", ID_GLOBAL);
            ASTptr assign = makeAssignStmt("__x", expr);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = assign;
            break;
        }
        case 9: {
            // valid assignment to local variable
            ASTptr v = makeVarDecl("a");
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v;

            ASTptr expr = makeIdentifier("__x", ID_GLOBAL);
            ASTptr assign = makeAssignStmt("a", expr);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = assign;
            break;
        }
        case 10: {
            // valid assigment to setter function
            ASTptr setter = makeFunction("foo", 1);
            setter->func.isSetter = true;
            setter->func.paramNames[0] = "v";
            program->program.funcs[program->program.funcsCount++] = setter;

            ASTptr expr = makeIdentifier("__x", ID_GLOBAL); // foo = __x
            ASTptr assign = makeAssignStmt("foo", expr);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = assign;
            break;
        }
        case 11: {
            // assignment to unknown variable
            ASTptr expr = makeIdentifier("__x", ID_GLOBAL);
            ASTptr assign = makeAssignStmt("x", expr);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = assign;
            break;
        }
        case 12: {
            // call non existing function
            ASTptr call = malloc(sizeof(ASTnode));
            call->type = AST_FUNC_CALL;
            call->call.funcName = "foo";
            call->call.argCount = 0;
            call->call.args = NULL;

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = call;
            break;
        }
        case 13: {
            // call function with wrong arity
            ASTptr foo = makeFunction("foo", 2);
            foo->func.paramNames[0] = "a";
            foo->func.paramNames[1] = "b";
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr call = malloc(sizeof(ASTnode));
            call->type = AST_FUNC_CALL;
            call->call.funcName = "foo";
            call->call.argCount = 1;
            call->call.args = malloc(sizeof(ASTptr));
            call->call.args[0] = makeIdentifier("__x", ID_GLOBAL);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = call;
            break;
        }
        case 14: {
            // call getter with arguments
            ASTptr foo = makeFunction("foo", 0);
            foo->func.isGetter = true;
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr call = malloc(sizeof(ASTnode));
            call->type = AST_FUNC_CALL;
            call->call.funcName = "foo";
            call->call.argCount = 1;
            call->call.args = malloc(sizeof(ASTptr));
            call->call.args[0] = makeIdentifier("__x", ID_GLOBAL);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = call;
            break;
        }
        case 15: {
            // call setter function
            ASTptr foo = makeFunction("foo", 1);
            foo->func.isSetter = true;
            foo->func.paramNames[0] = "v";
            program->program.funcs[program->program.funcsCount++] = foo;

            // foo(123) → invalid
            ASTptr call = malloc(sizeof(ASTnode));
            call->type = AST_FUNC_CALL;
            call->call.funcName = "foo";
            call->call.argCount = 1;
            call->call.args = malloc(sizeof(ASTptr));
            call->call.args[0] = makeIdentifier("__x", ID_GLOBAL);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = call;
            break;
        }
        case 16: {
            // valid function call
            ASTptr foo = makeFunction("foo", 2);
            foo->func.paramNames[0] = "a";
            foo->func.paramNames[1] = "b";
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr call = malloc(sizeof(ASTnode));
            call->type = AST_FUNC_CALL;
            call->call.funcName = "foo";
            call->call.argCount = 2;
            call->call.args = malloc(sizeof(ASTptr) * 2);
            call->call.args[0] = makeIdentifier("__x", ID_GLOBAL);
            call->call.args[1] = makeIdentifier("__y", ID_GLOBAL);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = call;
            break;
        }
        case 17: {
            // valid getter call
            ASTptr foo = makeFunction("foo", 0);
            foo->func.isGetter = true;
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr id = makeIdentifier("foo", ID_GETTER);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 18: {
            // valid use of identifier
            ASTptr v = makeVarDecl("a");
            ASTptr id = makeIdentifier("a", ID_LOCAL);

            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = v;
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 19: {
            // identifier does not exist
            ASTptr id = makeIdentifier("a", ID_LOCAL);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 20: {
            // global identifier usage
            ASTptr id = makeIdentifier("__x", ID_GLOBAL);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 21: {
            // getter that does not exist
            ASTptr id = makeIdentifier("foo", ID_GETTER);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 22: {
            // getter but it is a function
            ASTptr foo = makeFunction("foo", 2);
            foo->func.paramNames[0] = "x";
            foo->func.paramNames[1] = "y";
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr id = makeIdentifier("foo", ID_GETTER);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 23: {
            // valid getter
            ASTptr foo = makeFunction("foo", 0);
            foo->func.isGetter = true;
            program->program.funcs[program->program.funcsCount++] = foo;

            ASTptr id = makeIdentifier("foo", ID_GETTER);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = id;
            break;
        }
        case 24: {
            // valid string binary operation
            ASTptr l = makeLiteralString("abcd");
            ASTptr r = makeLiteralString("efgh");
            ASTptr bin = makeBinOp(BINOP_ADD, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        case 25: {
            // invalid string-number binary operation
            ASTptr l = makeLiteralString("ABC");
            ASTptr r = makeLiteralNumber(4);
            ASTptr bin = makeBinOp(BINOP_ADD, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        case 26: {
            // valid number binary operation
            ASTptr l = makeLiteralNumber(10);
            ASTptr r = makeLiteralNumber(3);
            ASTptr bin = makeBinOp(BINOP_SUB, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        case 27: {
            // invalid number-string binary operation
            ASTptr l = makeLiteralNumber(5);
            ASTptr r = makeLiteralString("abc");
            ASTptr bin = makeBinOp(BINOP_SUB, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        case 28: {
            // valid is operation
            ASTptr l = makeLiteralNumber(5);
            ASTptr r = makeLiteralNumber(10);
            ASTptr bin = makeBinOp(BINOP_IS, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        case 29: {
            // division by zero (in semantic its okay, runtime error)
            ASTptr l = makeLiteralNumber(4);
            ASTptr r = makeLiteralNumber(0);
            ASTptr bin = makeBinOp(BINOP_DIV, l, r);
            mainFunc->func.body->block.stmt[mainFunc->func.body->block.stmtCount++] = bin;
            break;
        }
        default:
            break;
    }
    
    semantic(program);
    return 0;
}

// ------------------------------------------ //

ASTptr makeProgram(){
    ASTptr program = malloc(sizeof(ASTnode));
    program->type = AST_PROGRAM;
    program->program.funcsCount = 0;
    program->program.funcsCap = 12;
    program->program.funcs = malloc(sizeof(ASTptr) * program->program.funcsCap);
    return program;
}

ASTptr makeFunction(char *name, int paramCount){
    ASTptr func = malloc(sizeof(ASTnode));
    func->type = AST_FUNC_DEF;
    func->func.name = name;
    func->func.paramCount = paramCount;
    func->func.paramNames = malloc(sizeof(char*) * paramCount);
    func->func.body = makeBlock();
    func->func.isGetter = false;
    func->func.isSetter = false;
    return func;
}

ASTptr makeBlock(){
    ASTptr block = malloc(sizeof(ASTnode));
    block->type = AST_BLOCK;
    block->block.stmtCount = 0;
    block->block.stmtCap = 12;
    block->block.stmt = malloc(sizeof(ASTptr) * block->block.stmtCap);
    return block;
}

ASTptr makeVarDecl(char *varName){
    ASTptr varDecl = malloc(sizeof(ASTnode));
    varDecl->type = AST_VAR_DECL;
    varDecl->var_decl.varName = varName;
    return varDecl;
}

ASTptr makeAssignStmt(char *targetName, ASTptr expr){
    ASTptr assignStmt = malloc(sizeof(ASTnode));
    assignStmt->type = AST_ASSIGN_STMT;
    assignStmt->assign_stmt.targetName = targetName;
    assignStmt->assign_stmt.expr = expr;
    return assignStmt;
}

ASTptr makeIdentifier(char *name, IdType idType){
    ASTptr id = malloc(sizeof(ASTnode));
    id->type = AST_IDENTIFIER;
    id->identifier.name = name;
    id->identifier.idType = idType;
    return id;
}

ASTptr makeLiteralString(char *value){
    ASTptr lit = malloc(sizeof(ASTnode));
    lit->type = AST_LITERAL;
    lit->literal.liType = LIT_STRING;
    lit->literal.str = value;
    return lit;
}

ASTptr makeLiteralNumber(double value){
    ASTptr lit = malloc(sizeof(ASTnode));
    lit->type = AST_LITERAL;
    lit->literal.liType = LIT_NUMBER;
    lit->literal.num = value;
    return lit;
}

ASTptr makeBinOp(BinOpType opType, ASTptr left, ASTptr right){
    ASTptr bin = malloc(sizeof(ASTnode));
    bin->type = AST_BINOP;
    bin->binop.opType = opType;
    bin->binop.left = left;
    bin->binop.right = right;
    return bin;
}