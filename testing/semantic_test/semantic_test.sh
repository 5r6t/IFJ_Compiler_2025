#!/bin/bash

#//////////////////////////////////////////////
#// filename: semantic_test.sh	             //
#// IFJ_prekladac	varianta - vv-BVS   	 //
#// Authors:						  		 //
#//  * Jaroslav Mervart (xmervaj00) / 5r6t 	 //
#//  * Veronika Kubová (xkubovv00) / Veradko //
#//  * Jozef Matus (xmatusj00) / karfisk 	 //
#//  * Jan Hájek (xhajekj00) / Wekk 		 //
#//////////////////////////////////////////////
# first use make to compile the semantic_test binary
# then run this script to execute semantic tests

BIN='./semantic_test'

GREEN="\033[32m"; 
RED="\033[31m"; 
YELLOW="\033[33m"; 
RESET="\033[0m"

run_test() {
    testNum=$1
    expectCode=$2

    $BIN $testNum >/dev/null 2>&1
    ret=$?

    if [ "$ret" -eq "$expectCode" ]; then
        echo -e "${GREEN}[PASS]${NC} Test $testNum (expected $expectCode, got $ret)${RESET}"
    else
        echo -e "${RED}[FAIL]${NC} Test $testNum (expected $expectCode, got $ret)${RESET}"
        FAIL=1
    fi
}

FAIL=0

echo "Function declaration tests:"
run_test 1 3   # missing main
run_test 2 4   # function has global variable as argument
run_test 3 4   # function has redefined argument
run_test 4 3   # unknown variable in assignment
echo "------------------------"
echo "Variable declaration tests:"
run_test 5 4   # declaring variable as global
run_test 6 4   # variable redeclaration
run_test 7 0   # correct variable declaration
echo "------------------------"
echo "Assignment tests:"
run_test 8 0  # valid assignment to global variable
run_test 9 0  # valid assignment to local variable
run_test 10 0 # valid assignment to setter
run_test 11 3 # assignment to undeclared variable
echo "------------------------"
echo "Function call tests:"
run_test 12 3  # call non existing function
run_test 13 5  # call with wrong number of arguments
run_test 14 5  # call getter with arguments
run_test 15 3  # setter called as function
run_test 16 0  # valid function call
run_test 17 0  # valid getter call
echo "------------------------"
echo "Identifier tests:"
run_test 18 0  # valid use of identifier
run_test 19 3  # identifier does not exist
run_test 20 0  # global identifier used
run_test 21 3  # getter that does not exist
run_test 22 3  # getter but it is a function
run_test 23 0  # valid getter
echo "------------------------"
echo "Binary operation tests:"
run_test 24 0  # valid string binary operation
run_test 25 6  # invalid string-number binary operation
run_test 26 0  # valid number binary operation
run_test 27 6  # invalid number-string binary operation
run_test 28 0  # valid is operation
run_test 29 0  # invalid number multiplication




if [ "$FAIL" -eq 0 ]; then
    echo -e "${GREEN}All semantic tests passed${RESET}"
    exit 0
else
    echo -e "${RED}Some semantic tests failed${RESET}"
    exit 1
fi