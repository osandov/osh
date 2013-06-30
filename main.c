#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

#include "tokenizer.h"

#define PS1 "$ "
#define DEBUG_TOKENS

#if 0
int exec_external(int argc, char **argv)
{
    pid_t pid = fork();
    if (pid == -1)
        error(1, errno, "fatal error");
    else if (pid) {
        int status;
        if (waitpid(pid, &status, 0) == -1)
            error(1, errno, "fatal error");
    } else {
        if ((execvp(argv[0], argv)) == -1)
            error(0, errno, "error");
        exit(127);
    }
    return -1;
}

void handle_line(char **tokens, size_t tokens_read)
{
    if (exec_builtin(tokens_read, tokens) == -1)
        exec_external(tokens_read, tokens);
}
#endif

int main(int argc, char **argv)
{
    char *line = NULL;
    struct token *tokens = NULL;
    size_t line_len = 0, tokens_len = 0;
    ssize_t line_read, tokens_read;

    printf("%s", PS1);
    while ((line_read = getline(&line, &line_len, stdin)) != -1) {
        if ((tokens_read = tokenize(&tokens, &tokens_len, line)) != -1) {
#ifdef DEBUG_TOKENS
            printf("[");
            if (tokens_read) {
                printf("'%s'", tokens[0].token);
                for (int i = 1; i < tokens_read; ++i)
                    printf(", '%s'", tokens[i].token);
            }
            printf("]\n");
#endif
#if 0
            if (tokens_read && tokens[0].token[0] != '#') {
                handle_line(tokens, tokens_read);
            }
#endif
        }
        printf("%s", PS1);
    }
    printf("\n");
    free(line);
    free(tokens);
    return 0;
}
