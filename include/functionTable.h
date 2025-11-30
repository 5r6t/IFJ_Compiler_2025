//////////////////////////////////////////////
// filename: functionTable.h          	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#ifndef FUNC_TABLE
#define FUNC_TABLE

#define MAX_FUNCTIONS 10
#define INBUILD_FUNCTIONS 10

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct ASTnode;
typedef struct ASTnode *ASTptr;


typedef enum {
    FUNC_NORMAL,
    FUNC_GETTER,
    FUNC_SETTER,
    FUNC_BUILTIN
} FuncKind;

typedef enum {
    PARAM_TYPE_NUMBER,
    PARAM_TYPE_STRING,
    PARAM_TYPE_ANY,
    PARAM_TYPE_NONE
} FuncParamType;

typedef struct FuncInfo {
    char *name;
    int paramCount;
    FuncParamType *paramTypes; // array of parameter types
    FuncKind kind;
    ASTptr funcNode;
} FuncInfo;

typedef struct {
    FuncInfo *functions; // dynamic array of functions (realloc)
    int funcCount;
    int funcCap;
} FuncTable;

void funcTableInit(FuncTable *table);
void funcTableFill(FuncTable *table);
void funcTableResize(FuncTable *table);
FuncInfo *funcTableGet(FuncTable *table, const char *name, const int paramCount);
FuncInfo *funcTableGetSetter(FuncTable *table, const char *name);
FuncInfo *funcTableGetGetter(FuncTable *table, const char *name);
bool funcTableAdd(FuncTable *table, FuncInfo func);
void funcTableFree(FuncTable *table);

char *myStrdup(char *str);

#endif // FUNC_TABLE