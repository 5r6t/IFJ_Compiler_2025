//////////////////////////////////////////////
// filename: codegen.c                      //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//////////////////////////////////////////////

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

typedef enum {
/* STACK OPERATORS */
    MOVE,           // <var> <symb>
    CREATEFRAME,
    PUSHFRAME,
    POPFRAME,       
    DEFVAR,         // DEFVAR <var>
    CALL,           // CALL <label>
    RETURN,

/* DATA STACK OPERATORS */
    PUSHS,  // PUSH <symb> save value <symb> to the top of the data stack
    POPS,   // POPS <var>  save top stack value to <var>, empty stack = err 56
    CLEAR,  // helper, clear the entire data stack

/* ARITHMETICAL OPERATORS */
    /// @note all use INSTR <var> <symb1> <symb2>
    ADD,
    SUB,
    MUL,
    /// @note null division causes err 57
    DIV,
    IDIV,

    /// @note stack versions of relation operands
    ADDS,
    SUBS,
    MULS,
    DIVS,
    IDIVS,

/* RELATION OPERATORS */
    // bool result to var
    // nil can only be compared with eq, else error 53
    LT,     // <var> <symb_1> <symb_2>
    GT,     // <var> <symb_1> <symb_2>
    EQ,     // <var> <symb_1> <symb_2>

    /// @note stack versions of relation operands
    LTS,
    GTS,
    EQS,

/* BOOL OPERATORS */
    // bool result to <var>
    AND,    // AND <var> <symb_1> <symb_2>
    OR,     // OR  <var> <symb_1> <symb_2>
    NOT,    // NOT <var> <symb>

    /// @note stack versions of bool operands
    ANDS,
    ORS,
    NOTS,

/* CONVERSION OPERATORS */
    INT2FLOAT,  // INT2FLOAT <var> <symb> convert <symb> to float, save to <var>
    FLOAT2INT,  // FLOAT2INT <var> <symb> convert <symb> to int, save to <var>
    INT2CHAR,   // INT2CHAR  <var> <symb> convert <symb> e <0;255> to ASCII, save to <var> else err 58
    STRI2INT,   // STRI2INT  <var> <symb_1> <symb_2> 
    // saves ASCII val of string <symb_1> char on pos <symb_2> to var. Out of range = err 58  

    FLOAT2STR,  // FLOAT2STR <var> <symb>
    // Ifj.str behaviour
    // convert float <symb> to string and save to var.

    INT2STR,    // INT2STR   <var> <symb>
    // Ifj.str behaviour
    // convert int <symb> to string and save to var.

    /// @note stack versions of conversion functions 
    INT2FLOATS,
    FLOAT2INTS,
    INT2CHARS,
    STRI2INTS,
    FLOAT2STRS,
    INT2STRS,

/* INPUT/OUTPUT OPERATORS */
    READ,  // READ  <var> <type> read, format as <type>, store in <var>
    // compatible with Ifj.read_str, Ifj.read_num, Ifj.read_bool

    WRITE, // WRITE <symb> to STDIO
    // compatible with Ifj.write

/* STRING OPERATORS */
    CONCAT, // CONCAT <var> <symb_1> <symb_2>
    // save to <var> concatenation of <symb_1> and <symb_2>; other types aren't allowed 

    STRLEN, // STRLEN <var> <symb>
    // determine length of string, save as int to <var>

    GETCHAR, // GETCHAR <var> <symb_1> <symb_2>
    // save one char string to <var> from string <symb_1> at pos <symb_2>    

    SETCHAR, // SETCHAR <var> <symb_1> <symb_2>
    // <var> string to replace character in
    // <symb_1> pos of char in string 
    // <symb_2> second string(it's first char)
    // <symb_1> not int => err 53, <symb_1> out of range || <symb_2> empty = err 58

/* TYPE OPERATORS */
    TYPE,   // TYPE  <var> <symb> 
    // determines <symb> type and writes string to <var> (int, bool, float, string, nil)
    // uninitialized variable is marked with an empty string

    ISINT,  // ISINT <var> <symb>
    // <symb> has to be either int or float
    // determines if <symb> is int or float, result written as bool into <var>
    
    /// @note stack versions of TYPE/ISINT (args come from stack)
    TYPES,  // TYPES 
    ISINTS, // ISINTS

/* FLOW operators */
    LABEL,      // LABEL <label> 
    // marks important position in the code as a potential target for jmp instructions
    // attempt to redefine results in err 52

    JUMP,       // JUMP  <label> jumps to designated label
    JUMPIFEQ,   // JUMPIFEQ <label> <symb_1> <symb_2>
    JUMPIFNEQ,  // JUMPIFEQ <label> <symb_1> <symb_2>

    /// @note stack versions of JUMPIFEQ/JUMPIFNEQ (args come from stack)
    JUMPIFEQS,  // JUMPIFEQS  <label>
    JUMPIFNEQS, // JUMPIFNEQS <label>

    EXIT,       // EXIT <symb> 
    // finish execution and exit with code <symb>, where symb is a number in <0,49>. 
    // Invalid return code results in err 57

/* DEBUG OPERATORS */ 
    // can be turned off with interpreter flags
    BREAK,      // prints interpreter state to STDERR
    DPRINT,     // DPRINT <symb> prints value <symb> to STDERR. 
}OpCode;

// for debug prints
#define TAC_OPCODE_LIST(OP) \
    OP(MOVE) \
    OP(CREATEFRAME) \
    OP(PUSHFRAME) \
    OP(POPFRAME) \
    OP(DEFVAR) \
    OP(CALL) \
    OP(RETURN) \
    OP(PUSHS) \
    OP(POPS) \
    OP(CLEAR) \
    OP(ADD) \
    OP(SUB) \
    OP(MUL) \
    OP(DIV) \
    OP(IDIV) \
    OP(ADDS) \
    OP(SUBS) \
    OP(MULS) \
    OP(DIVS) \
    OP(IDIVS) \
    OP(LT) \
    OP(GT) \
    OP(EQ) \
    OP(LTS) \
    OP(GTS) \
    OP(EQS) \
    OP(AND) \
    OP(OR) \
    OP(NOT) \
    OP(ANDS) \
    OP(ORS) \
    OP(NOTS) \
    OP(INT2FLOAT) \
    OP(FLOAT2INT) \
    OP(INT2CHAR) \
    OP(STRI2INT) \
    OP(FLOAT2STR) \
    OP(INT2STR) \
    OP(INT2FLOATS) \
    OP(FLOAT2INTS) \
    OP(INT2CHARS) \
    OP(STRI2INTS) \
    OP(FLOAT2STRS) \
    OP(INT2STRS) \
    OP(READ) \
    OP(WRITE) \
    OP(CONCAT) \
    OP(STRLEN) \
    OP(GETCHAR) \
    OP(SETCHAR) \
    OP(TYPE) \
    OP(ISINT) \
    OP(TYPES) \
    OP(ISINTS) \
    OP(LABEL) \
    OP(JUMP) \
    OP(JUMPIFEQ) \
    OP(JUMPIFNEQ) \
    OP(JUMPIFEQS) \
    OP(JUMPIFNEQS) \
    OP(EXIT) \
    OP(BREAK) \
    OP(DPRINT)

typedef struct TACnode {
    OpCode instr;
    char *a1;
    char *a2;
    char *a3;

    struct TACnode *prev;
    struct TACnode *next;
}TACnode;

typedef struct {
    TACnode *head;
    TACnode *tail;
}TAClist;

void print_tac(TAClist list);
TACnode *tac_append(OpCode instr, char *a1, char *a2, char *a3);
TACnode *tac_list_append(TAClist *list, OpCode instr, const char *a1, const char *a2, const char *a3);
TACnode *tac_list_pop_front(TAClist *list);

void tac_list_init(TAClist *list);
bool tac_list_is_empty(const TAClist *list);
void tac_list_remove(TAClist *list, TACnode *node);
void tac_node_free(TACnode *node);
void tac_list_clear(TAClist *list);
void tac_print_list_state(const char *label, const TAClist *list);
void tac_print_node(const char *label, const TACnode *node);

char* gen_identifier(ASTptr node);
char* gen_literal(ASTptr node);
char* fnc_name(const char* name);

void generate(ASTptr tree);
void gen_program(ASTptr node);
void gen_func_def(ASTptr node);
void gen_func_call(ASTptr node);
void gen_block(ASTptr node);
void gen_if_stmt(ASTptr node);
void gen_return_stmt(ASTptr node, int scopeDepth);
void gen_assign_stmt(ASTptr node);
void gen_stmt(ASTptr node);
char *gen_expr(ASTptr node);

#endif // CODEGEN_H
