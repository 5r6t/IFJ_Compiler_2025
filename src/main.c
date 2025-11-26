#include <stdio.h>
#include "lex.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

int main(int argc, char **argv)
{
    FILE *input = stdin;
    if (argc > 1)
    {
        input = fopen(argv[1], "r"); // optionally allows files as well
        if (input == NULL)
        {
            fprintf(stderr, "Unable to open %s\n", argv[1]);
            return 1;
        }
    }

    ASTptr root = parser(input); // parser calls lexer
    if (input != stdin)
    {
        fclose(input);
    }

    /*if (root == NULL) {
        return 1;
    }*/

    if (root != NULL)
    {
        fprintf(stderr, "===== AST DUMP =====\n");
        astPrint(root, 0);
        fprintf(stderr, "===== END =====\n");
    }
    else
    {
        return 1;
    }

    semantic(root);
    generate(root);

    return 0;
}
