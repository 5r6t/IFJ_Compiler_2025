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
    table->funcCap = MAX_FUNCTIONS+INBUILD_FUNCTIONS;
    table->functions = malloc((table->funcCap) * sizeof(FuncInfo));
}

/**
 * @brief fills the function table with inbuilt functions
 * 
 * @param table pointer to the function table
 */
void funcTableFill(FuncTable *table){
    char names[][16] = {"Ifj.read_str", "Ifj.read_num", "Ifj.write", "Ifj.floor", "Ifj.str", "Ifj.length", "Ifj.substring", "Ifj.strcmp", "Ifj.ord", "Ifj.chr"};
    int paramCounts[] = {0, 0, 1, 1, 1, 1, 3, 2, 2, 1};
    FuncParamType paramTypes[][3] = {
        {PARAM_TYPE_NONE}, // read_str
        {PARAM_TYPE_NONE}, // read_num
        {PARAM_TYPE_ANY}, // write
        {PARAM_TYPE_NUMBER}, // floor
        {PARAM_TYPE_ANY}, // str
        {PARAM_TYPE_STRING}, // length
        {PARAM_TYPE_STRING, PARAM_TYPE_NUMBER, PARAM_TYPE_NUMBER}, // substring
        {PARAM_TYPE_STRING, PARAM_TYPE_STRING}, // strcmp
        {PARAM_TYPE_STRING, PARAM_TYPE_NUMBER}, // ord
        {PARAM_TYPE_NUMBER}  // chr
    };

    for(int i = 0; i < INBUILD_FUNCTIONS; i++){
        FuncInfo func;
        func.name = myStrdup(names[i]);
        func.paramCount = paramCounts[i];
        func.paramTypes = malloc(func.paramCount * sizeof(FuncParamType));
        for(int j = 0; j < func.paramCount; j++){
            func.paramTypes[j] = paramTypes[i][j];
        }
        func.kind = FUNC_BUILTIN;
        func.funcNode = NULL;
        funcTableAdd(table, func);
    }
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
            if(table->functions[i].kind == FUNC_GETTER || table->functions[i].kind == FUNC_SETTER){
                continue; // skip getters and setters
            }
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
 * @brief retrieves a getter function from the function table by name
 * 
 * @param table pointer to the function table
 * @param name name of the function
 * @return FuncInfo* pointer to the function info if found, NULL otherwise
 */
FuncInfo *funcTableGetGetter(FuncTable *table, const char *name){
    for(int i = 0; i < table->funcCount; i++){
        if(strcmp(table->functions[i].name, name) == 0 && table->functions[i].kind == FUNC_GETTER){
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
    printf("Adding function: %s with %d parameters and kind %d\n", func.name, func.paramCount, func.kind);
    if(func.kind == FUNC_GETTER){
        if(funcTableGetGetter(table, func.name) != NULL){
            return false; // getter already exists
        }
    }else if(func.kind == FUNC_SETTER){
        if(funcTableGetSetter(table, func.name) != NULL){
            return false; // setter already exists
        }
    }else{
        if(funcTableGet(table, func.name, func.paramCount) != NULL){
            return false; // function already exists
        }
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

/**
 * @brief duplicates a string by allocating memory and copying the content
 * 
 * @param s input string to duplicate
 * @return pointer to the new string
 */
char *myStrdup(char *str){
    size_t len = strlen(str) + 1;
    char *new = malloc(len);
    if(new == NULL){
        return NULL;
    } 
    memcpy(new, str, len);
    return new;
}