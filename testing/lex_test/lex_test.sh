#!/bin/bash

#//////////////////////////////////////////////
#// filename: lex_test.sh	                 //
#// IFJ_prekladac	varianta - vv-BVS   	 //
#// Authors:						  		 //
#//  * Jaroslav Mervart (xmervaj00) / 5r6t 	 //
#//  * Veronika Kubová (xkubovv00) / Veradko //
#//  * Jozef Matus (xmatusj00) / karfisk 	 //
#//  * Jan Hájek (xhajekj00) / Wekk 		 //
#//////////////////////////////////////////////
#  Don't forget to run `chmod +x lex_test.sh`



TEST_BIN="../../build/test_lex_single"
GREEN="\033[32m"; RED="\033[31m"; YELLOW="\033[33m"; RESET="\033[0m"

$TEST_BIN "GROUP_COUNT" >/dev/null 2>&1
GROUP_LIMIT=$?  # See LEX_GROUP_COUNT in lex_test_single.h

ENV_LOC="../../.vscode/.env" # directory specific

# J.M's dedbug config // script updates the args
# {
#   "version": "0.2.0",
#   "configurations": [
#     {
#       "name": "Debug test",
#       "type": "cppdbg",
#       "request": "launch",
#       "program": "${workspaceFolder}/build/test_lex_single",
#       "args": [
#         "5",
#         "good",
#         "2"
#       ],
#       "cwd": "${workspaceFolder}",
#       "MIMode": "gdb",
#       "stopAtEntry": false
#     }
#   ]
# }

FAIL=false
fail_group=""
fail_type=""
fail_case=""

echo "========================================================"
echo " Running lexer test"
echo "========================================================"

for ((g=0; g<GROUP_LIMIT; g++)); do
    echo
    echo "===> Group $g"

    for type in good bad; do
        echo "  Running $type test cases..."
        i=0
        while true; do
            $TEST_BIN "$g" "$type" "$i" >/dev/null 2>&1
            code=$?

            # 100 -> group end, 98 -> invalid group
            if [ $code -eq 100 ] || [ $code -eq 98 ]; then
                break
            fi

            if [ "$type" = "good" ]; then
                # good cases: only return code 0 is pass
                if [ $code -eq 0 ]; then
                    printf "\t${GREEN}[PASS]${RESET} group %d good case %d\n" "$g" "$i"
                else
                    printf "\t${RED}[FAIL]${RESET} group %d good case %d\t(TEST exit %d)\n" "$g" "$i" "$code"
                    if [ "$FAIL" = false ]; then
                        FAIL=true
                        fail_case="$i"
                        fail_type="$type"
                        fail_group="$g"
                    fi
                fi
            else
                # bad cases: lexer exit(1) -> pass, 0 -> fail
                if [ $code -eq 1 ]; then
                    printf "\t${GREEN}[PASS]${RESET} group %d bad case %d\t(ERR_LEX)\n" "$g" "$i"
                else
                    printf "\t${RED}[FAIL]${RESET} group %d bad case %d\t(TEST exit %d)\n" "$g" "$i" "$code"
                    if [ "$FAIL" = false ]; then
                        FAIL=true
                        fail_case="$i"
                        fail_type="$type"
                        fail_group="$g"
                    fi
                fi
            fi
            ((i++))
        done
    done
done

echo
echo "========================================================"
echo " Lexer test complete"
echo "========================================================"

if [ "$FAIL" = false ]; then
    echo -e "${GREEN}>>> All test cases passed! ^w^${RESET}"
    exit 0
else
    while true; do
        echo
        echo "Do you want to run the first failed case individually? (select a number)"
        echo "Please use UpdateDBG only if you understand what it does"
        select yn in "Yes" "No" "Custom" "UpdateDBG"; do
            case $yn in
                "Yes")
                    echo "========================================================"
                    echo "Re-running test case..."
                    echo "========================================================"
                    $TEST_BIN "$fail_group" "$fail_type" "$fail_case"
                    break ;;

                "No")
                    break 2;;

                "Custom")
                    echo "========================================================"
                    echo " Re-running custom test case..."
                    echo "========================================================"
                    sleep 0.01
                    read -p "> Enter group index: " cg
                    read -p "> Enter type (good/bad): " ct
                    read -p "> Enter case index: " ci
                    echo "Running test case..."
                    $TEST_BIN "$cg" "$ct" "$ci"
                    break ;; 

                "UpdateDBG")
                    echo "Updating launch.json with failed case..."
                    LAUNCH_FILE="../../.vscode/launch.json"
                    tmpfile=$(mktemp)
                    sed -E 's,/\*.*\*/,,; s,//.*$,,' "$LAUNCH_FILE" | \
                        jq '.configurations[0].args = ["'"$fail_group"'", "'"$fail_type"'", "'"$fail_case"'"]' \
                        > "$tmpfile" && mv "$tmpfile" "$LAUNCH_FILE"
                    echo -e "${YELLOW}> launch.json updated — you can now F5 to debug${RESET}"
                    break 2;;
            esac
        done
    done
fi