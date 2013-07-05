#include <stdio.h>
#include <stdlib.h>

#include "cmdline.h"
#include "parser.h"
#include "tokenizer.h"

#define PS1 "$ "
/* #define DEBUG_TOKENS */
/* #define DEBUG_PARSER */

int main(int argc, char **argv)
{
    char *line = NULL;
    struct Token *tokens = NULL;
    size_t line_len = 0, tokens_len = 0;
    ssize_t line_read, tokens_read;

    printf("%s", PS1);
    while ((line_read = getline(&line, &line_len, stdin)) != -1) {
        if ((tokens_read = tokenize(&tokens, &tokens_len, line)) != -1) {
            struct SyntaxTree *tree = NULL;
#ifdef DEBUG_TOKENS
            print_tokens(tokens_read, tokens);
#endif
            tree = parse(tokens_read, tokens);
#ifdef DEBUG_PARSER
            print_tree(tree);
#endif
            if (tree) {
                exec_cmdline(tree);
                free_tree(tree);
            }
        }
        printf("%s", PS1);
    }
    printf("\n");
    free(line);
    free(tokens);
    return 0;
}
