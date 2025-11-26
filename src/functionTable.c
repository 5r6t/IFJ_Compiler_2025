//////////////////////////////////////////////
// filename: functionTable.c          	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "functionTable.h"

/**
 * @brief initializes the function table
 * 
 * @param table pointer to the function table
 */
void funcTableInit(FuncTable *table){
    table->funcCount = 0;
    table->funcCap = MAX_FUNCTIONS;
    table->functions = malloc(table->funcCap * sizeof(FuncInfo));
}

/**
 * @brief resizes the function table
 * 
 * @param table pointer to the function table
 */
void funcTableResize(FuncTable *table){
    table->funcCap *= 2;
    table->functions = realloc(table->functions, table->funcCap * sizeof(FuncInfo));
}

/**
 * @brief retrieves a function from the function table by name and parameter count
 * 
 * @param table pointer to the function table
 * @param name name of the function
 * @param paramCount number of parameters of the function
 * @return FuncInfo* pointer to the function info if found, NULL otherwise
 */
FuncInfo *funcTableGet(FuncTable *table, const char *name, const int paramCount){
    if(paramCount == -1){ // search without parameters
        for(int i = 0; i < table->funcCount; i++){
            if(strcmp(table->functions[i].name, name) == 0){
                return &table->functions[i];
            }
        }
    }else{
        for(int i = 0; i < table->funcCount; i++){
            if(strcmp(table->functions[i].name, name) == 0 && table->functions[i].paramCount == paramCount){
                return &table->functions[i];
            }
    }
    }
    return NULL;
}

/**
 * @brief retrieves a setter function from the function table by name
 * 
 * @param table pointer to the function table
 * @param name name of the function
 * @return FuncInfo* pointer to the function info if found, NULL otherwise
 */
FuncInfo *funcTableGetSetter(FuncTable *table, const char *name){
    for(int i = 0; i < table->funcCount; i++){
        if(strcmp(table->functions[i].name, name) == 0 && table->functions[i].kind == FUNC_SETTER){
            return &table->functions[i];
        }
    }
    return NULL;
}

/**
 * @brief adds a function to the function table
 * 
 * @param table pointer to the function table
 * @param func function info to add
 * @return true if the function was added successfully
 * @return false if the function already exists
 */
bool funcTableAdd(FuncTable *table, FuncInfo func){
    if(table->funcCount >= table->funcCap){
        funcTableResize(table);
    }
    if(funcTableGet(table, func.name, func.paramCount) != NULL){
        return false; // function already exists
    }
    table->functions[table->funcCount++] = func;
    return true;
}

/**
 * @brief frees the function table
 * 
 * @param table pointer to the function table
 */
void funcTableFree(FuncTable *table){
    free(table->functions);
    table->functions = NULL;
    table->funcCount = 0;
    table->funcCap = 0;
}