//////////////////////////////////////////////
// filename: common.c                  	    //
// IFJ_prekladac	varianta - vv-BVS   	//
// Authors:						  			//
//  * Jaroslav Mervart (xmervaj00) / 5r6t 	//
//  * Veronika Kubová (xkubovv00) / Veradko //
//  * Jozef Matus (xmatusj00) / karfisk 	//
//  * Jan Hájek (xhajekj00) / Wekk 			//
//////////////////////////////////////////////

#include "../include/common.h"
#include "../include/lex.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parser(FILE *file)
{
    TokenPtr nextToken = lexer(file); // lookahead -> maybe i shouldn`t declare nextToken here, something to think about
    if (PROGRAM(nextToken, file))
    {
        return 0;
    }
    else
    {
        return 1; // change to error code that is most fitting
    }
}

int PROGRAM(TokenPtr nextToken, FILE *file)
{
    if (PROLOG(nextToken, file))
    {
        if (CLASS(nextToken, file))
        {
            return 0;
        }
        else
        {
            return 1; // change to error code that is most fitting -> code will be 2. Should think about how to handle error better -> maybe use function here?( I wouldn`t need to have returned type of int)
        }
    }
    else
    {
        return 1; // change to error code that is most fitting
    }
}

int PROLOG(TokenPtr nextToken, FILE *file)
{
    // TODO change this to be more effective using function while_function
    char keyArray[5] = {
        "import",
        "\"ifj25\"",
        "for",
        "Ifj",
    };
    while_function(keyArray, file, nextToken);

    /*
    match("import");
    match("\"ifj_25\"");
    match("for");
    match("Ifj");
    */

    // here should be something to detected End Of Line (EOL)
    return 0;
}

int CLASS(TokenPtr nextToken, FILE *file)
{
    // TODO make while and array filled with "class Program { EOL". While would iterated until array is passted then function FUNCTIONS is called and after that check for }
    match("class", nextToken->data);
    nextToken = lexer(file); // lookahead
    match("Program", nextToken->data);
    nextToken = lexer(file);
    match("{", nextToken->data);
    nextToken = lexer(file);
    // here should be something to detected End Of Line (EOL)
    nextToken = lexer(file);
    FUNCTIONS(nextToken);
    nextToken = lexer(file);
    match("}", nextToken->data);
    return 0;
}

int FUNCTIONS(TokenPtr nextToken)
{
    if (match("static", nextToken->data))
    {
    }
}

// using strcmp compares target string with token. If token isn`t same as target string error is send to stderr output
int match(char target[], char token_data[])
{
    if (strcmp(target, token_data) != 0)
    {
        fprintf(stderr, "error code 2: syntax analyze error, expected: '%s', got '%s' \n", target, token_data);
        exit(2);
    }
    return 0;
}

// helping function for more pleasing way to check matches and for updating nextToken
void while_function(char targetStrArr[], FILE *file, TokenPtr nextToken)
{
    int i = 0;
    while (targetStrArr[i] == "\0")
    {
        match(targetStrArr[i], nextToken->data);
        nextToken = lexer(file);
        i++;
    }
}
