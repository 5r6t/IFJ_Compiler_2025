//////////////////////////////////////////////
// filename: lex_test_data.c                //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
// * Jaroslav Mervart (xmervaj00) / 5r6t 	//
// * Veronika Kubova (xkubovv00) / Veradko  //
// * Jozef Matus (xmatusj00) / karfisk 	    //
// * Jan Hajek (xhajekj00) / Wekk 	        //
//////////////////////////////////////////////

/**
 * @file lex_test_data.c
 * @brief Numeric literal lexer test data (valid + invalid cases)
 */

#include "../../include/lex.h"
#include "lex_test_data.h"
#include <stddef.h>

// Small helper to inline token sequences
#define TOK_SEQ(...) ((struct Token[]){__VA_ARGS__, {-1, NULL, NULL}}) // token and eof

/* ============================
 * GOOD cases
 * ============================ */
LexCase lex_good_num_cases[] = {
    { "0",      TOK_SEQ({NUMERICAL, NULL, "0"}) },
    { "0x0",    TOK_SEQ({NUMERICAL, NULL, "0x0"}) },
    { "0x1F",   TOK_SEQ({NUMERICAL, NULL, "0x1F"}) },
    { "0XABC",  TOK_SEQ({NUMERICAL, NULL, "0XABC"}) },
    { "123",    TOK_SEQ({NUMERICAL, NULL, "123"}) },
    { "123.0",  TOK_SEQ({NUMERICAL, NULL, "123.0"}) },
    { "123.01", TOK_SEQ({NUMERICAL, NULL, "123.01"}) },
    { "123e-5", TOK_SEQ({NUMERICAL, NULL, "123e-5"}) },
    { "123e05", TOK_SEQ({NUMERICAL, NULL, "123e05"}) },
    { "0.0",    TOK_SEQ({NUMERICAL, NULL, "0.0"}) },
    { "0.01",   TOK_SEQ({NUMERICAL, NULL, "0.01"}) },
    { "1.2e-3", TOK_SEQ({NUMERICAL, NULL, "1.2e-3"}) },
    { "1.2e03", TOK_SEQ({NUMERICAL, NULL, "1.2e03"}) },
    { "99999999999999999999", TOK_SEQ({NUMERICAL, NULL, "99999999999999999999"}) },

    { NULL, NULL }
};

LexCase lex_good_ident_cases[] = {
    /* identifiers */
    { "a",             TOK_SEQ({IDENTIFIER, NULL, "a"}) },
    { "_abc",          TOK_SEQ({IDENTIFIER, NULL, "_abc"}) },
    { "x123",          TOK_SEQ({IDENTIFIER, NULL, "x123"}) },

    /* global variables */
    { "__var",         TOK_SEQ({ID_GLOBAL_VAR, NULL, "__var"}) },
    { "__A1b2",        TOK_SEQ({ID_GLOBAL_VAR, NULL, "__A1b2"}) },
    { "__",            TOK_SEQ({ID_GLOBAL_VAR, NULL, "__"}) }, // allowed

    /* keywords */
    { "class",         TOK_SEQ({KW_CLASS, NULL, "class"}) },
    { "else",          TOK_SEQ({KW_ELSE, NULL, "else"}) },
    { "for",           TOK_SEQ({KW_FOR, NULL, "for"}) },
    { "if",            TOK_SEQ({KW_IF, NULL, "if"}) },
    { "Ifj",           TOK_SEQ({KW_IFJ, NULL, "Ifj"}) },
    { "import",        TOK_SEQ({KW_IMPORT, NULL, "import"}) },
    { "is",            TOK_SEQ({KW_IS, NULL, "is"}) },
    { "null",          TOK_SEQ({KW_NULL, NULL, "null"}) },
    { "Null",          TOK_SEQ({KW_NULL_TYPE, NULL, "Null"}) },
    { "Num",           TOK_SEQ({KW_NUM, NULL, "Num"}) },
    { "return",        TOK_SEQ({KW_RETURN, NULL, "return"}) },
    { "static",        TOK_SEQ({KW_STATIC, NULL, "static"}) },
    { "String",        TOK_SEQ({KW_STRING, NULL, "String"}) },
    { "var",           TOK_SEQ({KW_VAR, NULL, "var"}) },
    { "while",         TOK_SEQ({KW_WHILE, NULL, "while"}) },

    { NULL, NULL }
};

/* ============================
 * BAD cases
 * ============================ */
LexCase lex_bad_num_cases[] = {
    { "00",       TOK_SEQ({NUMERICAL, NULL, "00"}) },
    { "01",       TOK_SEQ({NUMERICAL, NULL, "01"}) },
    { "0x",       TOK_SEQ({NUMERICAL, NULL, "0x"}) },
    { "000123",   TOK_SEQ({NUMERICAL, NULL, "000123"}) },
    { "123.",     TOK_SEQ({NUMERICAL, NULL, "123."}) },
    { "123e",     TOK_SEQ({NUMERICAL, NULL, "123e"}) },
    { "123e+",    TOK_SEQ({NUMERICAL, NULL, "123e+"}) },
    { ".123",     TOK_SEQ({NUMERICAL, NULL, ".123"}) },
    { "1.2e",     TOK_SEQ({NUMERICAL, NULL, "1.2e"}) },
    { "1.2e+",    TOK_SEQ({NUMERICAL, NULL, "1.2e+"}) },
    { "1.e3",   TOK_SEQ({NUMERICAL, NULL, "1.e3"}) },
    
    { NULL, NULL }
};

LexCase lex_bad_ident_cases[] = {
    { "1abc",          TOK_SEQ({ERR_LEX, NULL, "1abc"}) },     // starts with digit
    { "#name",         TOK_SEQ({ERR_LEX, NULL, "#name"}) },    // invalid char
    { "$var",          TOK_SEQ({ERR_LEX, NULL, "$var"}) },     // illegal sigil
    { "_",             TOK_SEQ({ERR_LEX, NULL, "_"}) },        // underscore alone

    { "__9abc",        TOK_SEQ({ERR_LEX, NULL, "__9abc"}) },   // global var starting with digit after underscores
    { "var$",          TOK_SEQ({ERR_LEX, NULL, "var$"}) },     // illegal symbol at end
    { "abc-def",       TOK_SEQ({ERR_LEX, NULL, "abc-def"}) },  // hyphen not allowed
    { "a.b",           TOK_SEQ({ERR_LEX, NULL, "a.b"}) },      // dot not valid inside identifier

    { NULL, NULL }
};

/* ============================
 * Group registry
 * ============================ */
LexGroup lex_groups[] = {
    { "num", lex_good_num_cases, lex_bad_num_cases },
    { "ident", lex_good_ident_cases, lex_bad_ident_cases },

    { NULL, NULL, NULL } //
};

const int LEX_GROUP_COUNT = 2; // update for script to include new groups!!!