#include <stdio.h>
#include "lex.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

int main(int argc, char **argv) {
    FILE *input = stdin;
    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Unable to open %s\n", argv[1]);
            return 1;
        }
    }

    ASTptr root = parser(input); // parser calls lexer
    if (input != stdin) {
        fclose(input);
    }

    if (root == NULL) {
        return 1;
    }

    semantic(root);
    generate(root);

    return 0;
}
