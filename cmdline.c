#include <assert.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

#include "cmdline.h"

static inline char **to_argv(size_t num_tokens, struct Token *tokens)
{
    char **argv = malloc(sizeof(char*) * (num_tokens + 1));
    int i;
    for (i = 0; i < num_tokens; ++i)
        argv[i] = tokens[i].token;
    argv[i] = NULL;
    return argv;
}

static int exec_cmd(struct SyntaxTree *root);
static int exec_pipe(struct SyntaxTree *root, bool err_pipe);
static int exec_and(struct SyntaxTree *root);
static int exec_or(struct SyntaxTree *root);
static int exec_semicolon(struct SyntaxTree *root);
static int exec_background(struct SyntaxTree *root);

int exec_cmdline(struct SyntaxTree *root)
{
    switch (root->type) {
        case NODE_CMD:
            return exec_cmd(root);
        case NODE_PIPE:
            return exec_pipe(root, false);
        case NODE_ERR_PIPE:
            return exec_pipe(root, true);
        case NODE_AND:
            return exec_and(root);
        case NODE_OR:
            return exec_or(root);
        case NODE_SEMICOLON:
            return exec_semicolon(root);
        case NODE_BACKGROUND:
            return exec_background(root);
        default:
            assert(0);
    }
}

static int exec_cmd(struct SyntaxTree *root)
{
    int status;
    char **argv = to_argv(root->num_tokens, root->tokens);
    pid_t pid;
    if ((pid = fork()) == -1)
        error(1, errno, "error");
    if (pid) {
        if (waitpid(pid, &status, 0) == -1)
            error(1, errno, "error");
    } else {
        if (execvp(argv[0], argv) == -1)
            error(1, errno, "error");
    }
    return WEXITSTATUS(status);
}

static int exec_pipe(struct SyntaxTree *root, bool err_pipe)
{
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
        error(1, errno, "error");

    if ((pid1 = fork()) == -1)
        error(1, errno, "error");

    if (pid1) {
        if ((pid2 = fork()) == -1)
            error(1, errno, "error");
        if (pid2) {
            close(pipefd[0]);
            close(pipefd[1]);
        } else {
            dup2(pipefd[0], 0);
            close(pipefd[1]);
            exit(exec_cmdline(root->right));
        }
    } else {
        if (err_pipe)
            dup2(pipefd[1], 2);
        else
            dup2(pipefd[1], 1);
        close(pipefd[0]);
        exit(exec_cmdline(root->left));
    }

    bool done[2] = {false, false};
    int retval;
    while (!done[0] || !done[1]) {
        int status;
        pid_t pid = wait(&status);
        if (pid == pid1)
            done[0] = true;
        else if (pid == pid2) {
            done[1] = true;
            retval = WEXITSTATUS(status);
        }
    }

    return retval;
}

static int exec_and(struct SyntaxTree *root)
{
    int retval = exec_cmdline(root->left);
    if (retval == 0)
        retval = exec_cmdline(root->right);
    return retval;
}

static int exec_or(struct SyntaxTree *root)
{
    int retval = exec_cmdline(root->left);
    if (retval != 0)
        retval = exec_cmdline(root->right);
    return retval;
}

static int exec_semicolon(struct SyntaxTree *root)
{
    int retval = 0;
    if (root->left)
        retval = exec_cmdline(root->left);
    if (root->right)
        retval = exec_cmdline(root->right);
    return retval;
}

static int exec_background(struct SyntaxTree *root)
{
    int retval = 0;
    pid_t pid;
    if ((pid = fork()) == -1)
        error(1, errno, "error");
    if (pid) {
        if (root->right)
            retval = exec_cmdline(root->right);
    } else
        exit(exec_cmdline(root->left));
    return retval;
}
