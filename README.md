# ifj_2025
Formal Languages and Compilers project repo
IFJ Project by team "automats enjoyers" (team xmervaj00)

## Useful

[IFJ website](https://www.fit.vut.cz/study/course/IFJ/public/project/index.php.cz)

## To-Do

- [ ] choose assignment variant:
    - Assignment variant with the implementation of the symbol table using a table with scattered items with implicit chaining of items (TRP with open addressing)
    - AST variant - assignment with the implementation of the symbol table as a height-balanced binary search tree
- [ ] go through the assignment together
- [ ] try out the functionality of git etc. (e.g. everyone adds a few constants for error messages, see assignment page?)

## Team

> Jaroslav Mervart (xmervaj00) / 5r6t

> Veronika Kubová (xkubovv00) / Veradko

> Jozef Matus (xmatusj00) / karfisk

> Jan Hájek (xhajekj00) / Wekk

## DEADLINES

Code should be done by November 18. !!!

Submission of the entire project - 3.12.2025 
---
@repo created on 22.9.2024

## LL1 rules

/ is EOL

PROGRAM ::= PROLOG CLASS
PROLOG ::= import "ifj25" for Ifj /
CLASS ::= class Program { / FUNCTIONS } 
FUNCTIONS ::= static FUNC_NAME FUNC_GET_SET_DEF FUNCTIONS 
FUNCTIONS ::= ''
FUNC_GET_SET_DEF ::= ( PAR ) { / FUNC_BODY } / 
FUNC_GET_SET_DEF ::= { / FUNC_BODY } /
FUNC_GET_SET_DEF ::= = ( id ) { / FUNC_BODY } / 

PAR ::= '' 
PAR ::= id NEXT_PAR
NEXT_PAR ::= , id NEXT_PAR 
NEXT_PAR ::= ''

ARG ::= ''
ARG ::= ARG_NAME NEXT_ARG
NEXT_ARG ::= ''
NEXT_ARG ::= , ARG_NAME NEXT_ARG

ARG_NAME ::= int
ARG_NAME ::= string
ARG_NAME ::= float
ARG_NAME ::= null
ARG_NAME ::= id

EXPRESSION ::= int 
EXPRESSION ::= string
EXPRESSION ::= float
FUNC_NAME ::= id 
VAR_NAME ::= id
VAR_NAME ::= global_id

FUNC_BODY ::= '' 
FUNC_BODY ::= VAR_DECL FUNC_BODY 
FUNC_BODY ::= VAR_ASS_CALL_GET FUNC_BODY 	
FUNC_BODY ::= IF_STAT FUNC_BODY 
FUNC_BODY ::= WHILE FUNC_BODY 
FUNC_BODY ::= RETURN FUNC_BODY 

VAR_DECL ::= var VAR_NAME /
VAR_ASS_CALL_GET ::= VAR_NAME = RSA
RSA ::= EXPRESSION /
RSA ::= FUNC_NAME FUNC_TYPE /
FUNC_TYPE ::= ''
FUNC_TYPE ::= ( ARG ) 
IF_STAT ::= if ( EXPRESSION ) { / FUNC_BODY } / else { / FUNC_BODY } /
WHILE ::= while ( EXPRESSION ) { / FUNC_BODY } /
RETURN ::= return EXPRESSION /
