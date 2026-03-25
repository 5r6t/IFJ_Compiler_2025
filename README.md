# IFJ25 Compiler Project
## Useful links
- [Official IFJ website](https://www.fit.vut.cz/study/course/IFJ/public/project/index.php.cz)
- [Assignment](assignment/project_assignment_IFJ_a_IAL_2025.pdf) and [assignment section](assignment)
- [Documentation](documentation/ifj25_documentation.pdf)
- [Presentation](documentation/ifj25_compiler_presentation.pdf) and it's [accompanying text](documentation/ifj25_compiler_text_presentation.pdf)
## Team (xmervaj00)

> Jaroslav Mervart (xmervaj00) / 5r6t

> Veronika Kubová (xkubovv00) / Veradko

> Jozef Matus (xmatusj00) / karfisk

> Jan Hájek (xhajekj00) / Wekk
---
## To-Do

- [x] choose assignment variant:
    - Assignment variant with the implementation of the symbol table using a table with scattered items with implicit chaining of items (TRP with open addressing)
    - AST variant - assignment with the implementation of the symbol table as a height-balanced binary search tree
- [x] go through the assignment together
- [x] try out the git functionality, etc. (e.g. everyone adds a few constants for error messages, see assignment page?)

## Deadlines

- The coding part should be done by 18. 11. 2025
- Submission of the entire project - 3. 12. 2025

---
## Notes/Disclaimers
- Running `make` builds the compiler.
- Script [is_it_ok.sh](is_it_ok.sh) has been included with the assignment and belongs to its rightful owners. It is included for informational and testing purposes.
- Script [pack_it_up.sh](pack_it_up.sh) is not required to run the program and is used to archive relevant files into a directory (flattens directories, required by assignment). 
- Files [rozdeleni](rozdeleni) and [rozsireni](rozsireni) are irrelevant (used in grading process)

# Final evaluation
Original output:
```
Lexikální analýza (detekce chyb): 92 % (166/180 mb, špatné kódy 7 %)
Syntaktická analýza (detekce chyb): 98 % (245/250 mb, špatné kódy 2 %)
Sémantická analýza (detekce chyb): 77 % (162/210 mb, špatné kódy 22 %)
Sémantické/běhové chyby (detekce chyb): 100 % (160/160 mb, špatné kódy 0 %)
Interpretace přeloženého kódu (základní): 55 % (138/247 mb, špatné kódy 33 %, nesouhlasné výpisy 10 %)
Interpretace přeloženého kódu (výrazy, vest. funkce): 67 % (212/314 mb, špatné kódy 21 %, nesouhlasné výpisy 10 %)
Interpretace přeloženého kódu (komplexní): 50 % (238/471 mb, špatné kódy 49 %, nesouhlasné výpisy 0 %)
FUNEXP 0 % (0/150 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
EXTSTAT 0 % (0/25 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
EXTFUN 0 % (0/10 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
BOOLTHEN 0 % (0/100 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
CYCLES 12 % (12/100 mb, špatné kódy 88 %, nesouhlasné výpisy 0 %)
OPERATORS 0 % (0/25 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
ONELINEBLOCK 0 % (0/25 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
STATICAN 0 % (0/215 mb, špatné kódy 100 %, nesouhlasné výpisy 0 %)
Celkem bez rozšíření: 72 % (1321/1832 mb)
```
Formatted output:
| Component / Extension | Accuracy | Tested Cases | Wrong Codes | Mismatched Outputs |
|----------------------|---------|-------------|------------|------------------|
| Lexical analysis (error detection) | 92% | 166/180 | 7% | – |
| Syntax analysis (error detection) | 98% | 245/250 | 2% | – |
| Semantic analysis (error detection) | 77% | 162/210 | 22% | – |
| Semantic/runtime errors (detection) | 100% | 160/160 | 0% | – |
| Interpreted compiled code (basic) | 55% | 138/247 | 33% | 10% |
| Interpreted compiled code (expressions, built-in functions) | 67% | 212/314 | 21% | 10% |
| Interpreted compiled code (complex) | 50% | 238/471 | 49% | 0% |
| FUNEXP | 0% | 0/150 | 100% | 0% |
| EXTSTAT | 0% | 0/25 | 100% | 0% |
| EXTFUN | 0% | 0/10 | 100% | 0% |
| BOOLTHEN | 0% | 0/100 | 100% | 0% |
| CYCLES | 12% | 12/100 | 88% | 0% |
| OPERATORS | 0% | 0/25 | 100% | 0% |
| ONELINEBLOCK | 0% | 0/25 | 100% | 0% |
| STATICAN | 0% | 0/215 | 100% | 0% |
| Total without extensions | 72% | 1321/1832 | – | – |

## LL1 rules
```js
/ is EOL

- PROGRAM ::= PROLOG CLASS 
- PROLOG ::= import "ifj25" for Ifj /
- CLASS ::= class Program { / FUNCTIONS } 
- FUNCTIONS ::= static FUNC_NAME FUNC_GET_SET_DEF FUNCTIONS 
- FUNCTIONS ::= ''
- FUNC_GET_SET_DEF ::= ( PAR ) { / FUNC_BODY } / 
- FUNC_GET_SET_DEF ::= { / FUNC_BODY } /
- FUNC_GET_SET_DEF ::= = ( id ) { / FUNC_BODY } / 

- PAR ::= '' 
- PAR ::= id NEXT_PAR
- NEXT_PAR ::= , id NEXT_PAR 
- NEXT_PAR ::= ''

- ARG ::= ''
- ARG ::= ARG_NAME NEXT_ARG
- NEXT_ARG ::= ''
- NEXT_ARG ::= , ARG_NAME NEXT_ARG

- ARG_NAME ::= int
- ARG_NAME ::= string
- ARG_NAME ::= float
- ARG_NAME ::= null
- ARG_NAME ::= id

- EXPRESSION ::= int 
- EXPRESSION ::= string
- EXPRESSION ::= float
- FUNC_NAME ::= id 
- VAR_NAME ::= id
- VAR_NAME ::= global_id

- FUNC_BODY ::= '' 
- FUNC_BODY ::= VAR_DECL FUNC_BODY 
- FUNC_BODY ::= VAR_ASS_CALL_GET FUNC_BODY 	
- FUNC_BODY ::= IF_STAT FUNC_BODY 
- FUNC_BODY ::= WHILE FUNC_BODY 
- FUNC_BODY ::= RETURN FUNC_BODY 

- VAR_DECL ::= var VAR_NAME /
- VAR_ASS_CALL_GET ::= VAR_NAME = RSA
- RSA ::= EXPRESSION /
- RSA ::= FUNC_NAME FUNC_TYPE /
- FUNC_TYPE ::= ''
- FUNC_TYPE ::= ( ARG ) 
- IF_STAT ::= if ( EXPRESSION ) { / FUNC_BODY } / else { / FUNC_BODY } /
- WHILE ::= while ( EXPRESSION ) { / FUNC_BODY } /
- RETURN ::= return EXPRESSION /
```

Repository created: 16 September 2025
