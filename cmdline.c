#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

#include "cmdline.h"

static int exec_node(struct SyntaxTree *root);
static int exec_pipeline(size_t num_tokens, struct Token *tokens);

int exec_cmdline(struct SyntaxTree *root)
{
    return exec_node(root);
}

static int exec_node(struct SyntaxTree *root)
{
    int retval = 0;
    switch (root->type) {
        case NODE_PIPELINE:
            retval = exec_pipeline(root->num_tokens, root->tokens);
            break;
        case NODE_SEMICOLON:
            exec_node(root->left);
            retval = exec_node(root->right);
            break;
        case NODE_AND:
            retval = exec_node(root->left);
            if (!retval)
                retval = exec_node(root->right);
            break;
        case NODE_OR:
            retval = exec_node(root->left);
            if (retval)
                retval = exec_node(root->right);
            break;
        default:
            return -1;
    }
    return retval;
}

static int exec_pipeline(size_t num_tokens, struct Token *tokens)
{
    int argc = num_tokens;
    char **argv = malloc((argc + 1) * sizeof(char**));
    int status;
    for (size_t i = 0; i < argc; ++i)
        argv[i] = tokens[i].token;
    argv[argc] = NULL;
    
    pid_t pid = fork();
    if (pid == -1)
        error(1, errno, "fatal error");
    if (pid) {
        if (waitpid(pid, &status, 0) == -1)
            error(1, errno, "fatal error");
    } else {
        if (execvp(argv[0], argv) == -1)
            error(0, errno, "error");
        exit(127);
    }

    free(argv);
    return status;
}
