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

// Small helper for inline token sequences
#define TOK(t, i, d)  { .type = (t), .id = (i), .data = (d) }
#define TOK_SEQ(...) ((struct Token[]){__VA_ARGS__, {-1, NULL, NULL}}) // token and eof

/// @note data format { NULL,      TOK_SEQ( TOK("type", "id", "data") ) },  

/* ============================
 * GOOD cases
 * ============================ */
LexCase lex_good_num_cases[] = {
    { "0",      TOK_SEQ( TOK(NUMERICAL, NULL, "0") ) },
    { "0x0",    TOK_SEQ( TOK(NUMERICAL, NULL, "0x0") ) },
    { "0.1",    TOK_SEQ( TOK(NUMERICAL, NULL, "0.1") ) },
    { "0x1F",   TOK_SEQ( TOK(NUMERICAL, NULL, "0x1F") ) },
    { "0XABC",  TOK_SEQ( TOK(NUMERICAL, NULL, "0xABC") ) },
    { "123",    TOK_SEQ( TOK(NUMERICAL, NULL, "123") ) },
    { "123.0",  TOK_SEQ( TOK(NUMERICAL, NULL, "123.0") ) },
    { "123.01", TOK_SEQ( TOK(NUMERICAL, NULL, "123.01") ) },
    { "123e-5", TOK_SEQ( TOK(NUMERICAL, NULL, "123e-5") ) },
    { "123e05", TOK_SEQ( TOK(NUMERICAL, NULL, "123e05") ) },
    { "0.0",    TOK_SEQ( TOK(NUMERICAL, NULL, "0.0") ) },
    { "0e1",    TOK_SEQ( TOK(NUMERICAL, NULL, "0e1") ) },
    { "0.01",   TOK_SEQ( TOK(NUMERICAL, NULL, "0.01") ) },
    { "1.2e-3", TOK_SEQ( TOK(NUMERICAL, NULL, "1.2e-3") ) },
    { "1.2e03", TOK_SEQ( TOK(NUMERICAL, NULL, "1.2e03") ) },
    { NULL, NULL }
};

LexCase lex_good_ident_cases[] = {
    /* identifiers */
    { "a",       TOK_SEQ( TOK(IDENTIFIER, "a", NULL) ) },
    { "_abc",    TOK_SEQ( TOK(IDENTIFIER, "_abc", NULL) ) },
    { "x123",    TOK_SEQ( TOK(IDENTIFIER, "x123", NULL) ) },

    /* global variables */
    { "__var",   TOK_SEQ( TOK(ID_GLOBAL_VAR, "__var", NULL) ) },
    { "__A1b2",  TOK_SEQ( TOK(ID_GLOBAL_VAR, "__A1b2", NULL) ) },
    { "__",      TOK_SEQ( TOK(ID_GLOBAL_VAR, "__", NULL) ) },

    /* keywords */
    { "class",   TOK_SEQ( TOK(KW_CLASS, "class", NULL) ) },
    { "else",    TOK_SEQ( TOK(KW_ELSE, "else", NULL) ) },
    { "for",     TOK_SEQ( TOK(KW_FOR, "for", NULL) ) },
    { "if",      TOK_SEQ( TOK(KW_IF, "if", NULL) ) },
    { "Ifj",     TOK_SEQ( TOK(KW_IFJ, "Ifj", NULL) ) },
    { "import",  TOK_SEQ( TOK(KW_IMPORT, "import", NULL) ) },
    { "is",      TOK_SEQ( TOK(KW_IS, "is", NULL) ) },
    { "null",    TOK_SEQ( TOK(KW_NULL, NULL, "null") ) }, // null is value -> data
    { "Null",    TOK_SEQ( TOK(KW_NULL_TYPE, "Null", NULL) ) },
    { "Num",     TOK_SEQ( TOK(KW_NUM, "Num", NULL) ) },
    { "return",  TOK_SEQ( TOK(KW_RETURN, "return", NULL) ) },
    { "static",  TOK_SEQ( TOK(KW_STATIC, "static", NULL) ) },
    { "String",  TOK_SEQ( TOK(KW_STRING, "String", NULL) ) },
    { "var",     TOK_SEQ( TOK(KW_VAR, "var", NULL) ) },
    { "while",   TOK_SEQ( TOK(KW_WHILE, "while", NULL) ) },

    { NULL, NULL }
};

LexCase lex_good_cmp_cases[] = {
    { ">",       TOK_SEQ( TOK(CMP_OPERATOR, ">", NULL) ) },
    { "<",       TOK_SEQ( TOK(CMP_OPERATOR, "<", NULL) ) },
    { ">=",      TOK_SEQ( TOK(CMP_OPERATOR, ">=", NULL ) ) },
    { "<=",      TOK_SEQ( TOK(CMP_OPERATOR, "<=", NULL) ) },
    { "==",      TOK_SEQ( TOK(CMP_OPERATOR, "==", NULL) ) },
    { "!=",      TOK_SEQ( TOK(CMP_OPERATOR, "!=", NULL) ) },

    { NULL, NULL }
};


/* ============================
 * BAD cases
 * ============================ */
LexCase lex_bad_num_cases[] = {
    { "00",       TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "01",       TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "1xabc",    TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "123.",     TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "123e",     TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "123e+",    TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { ".123",     TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "1.2e",     TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "1.2e+",    TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { "1.e3",     TOK_SEQ( TOK(NUMERICAL, NULL, NULL) ) },
    { NULL, NULL }
};


LexCase lex_bad_ident_cases[] = {
    { "1abc",     NULL },     // starts with digit
    { "#name",    NULL },    // invalid char
    { "$var",     NULL },     // illegal sigil
    { "_",        NULL },        // underscore alone

    { "__9abc",   NULL },   // global var starting with digit after underscores
    { "var$",     NULL },     // illegal symbol at end

    { NULL, NULL }
};

LexCase lex_bad_cmp_cases[] = {
    { ">>",      NULL },
    { "<<",      NULL },
    { "=>",      NULL},
    { "=<",      NULL },
    { "=!",      NULL },

    { NULL, NULL }
};
/* ============================
 * Group registry
 * ============================ */
LexGroup lex_groups[] = {
    { "num", lex_good_num_cases, lex_bad_num_cases },
    { "ident", lex_good_ident_cases, lex_bad_ident_cases },
    { "cmp", lex_good_cmp_cases, lex_bad_cmp_cases},

    { NULL, NULL, NULL }
};