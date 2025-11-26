//////////////////////////////////////////////
// filename: functionTable.h          	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#ifndef FUNC_TABLE
#define FUNC_TABLE

#define MAX_FUNCTIONS 10

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct ASTnode;
typedef struct ASTnode *ASTptr;


typedef enum {
    FUNC_NORMAL,
    FUNC_GETTER,
    FUNC_SETTER
} FuncKind;

typedef struct FuncInfo {
    char *name;
    int paramCount;
    FuncKind kind;
    ASTptr funcNode;
} FuncInfo;

typedef struct {
    FuncInfo *functions; // dynamic array of functions (realloc)
    int funcCount;
    int funcCap;
} FuncTable;

void funcTableInit(FuncTable *table);
void funcTableResize(FuncTable *table);
FuncInfo *funcTableGet(FuncTable *table, const char *name, const int paramCount);
FuncInfo *funcTableGetSetter(FuncTable *table, const char *name);
bool funcTableAdd(FuncTable *table, FuncInfo func);
void funcTableFree(FuncTable *table);

#endif // FUNC_TABLE