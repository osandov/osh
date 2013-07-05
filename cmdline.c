#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

#include "builtin.h"
#include "error.h"
#include "cmdline.h"

static int exec_cmd(struct SyntaxTree *root);
static int exec_redir_in(struct SyntaxTree *root);
static int exec_redir_out(struct SyntaxTree *root, bool append);
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
        case NODE_REDIR_IN:
            return exec_redir_in(root);
        case NODE_REDIR_OUT:
            return exec_redir_out(root, false);
        case NODE_REDIR_APPEND:
            return exec_redir_out(root, true);
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
        case NODE_DISOWN:
            assert(0);
    }
    return -1;
}

static int exec_cmd(struct SyntaxTree *root)
{
    int retval;

    char **argv = malloc(sizeof(char*) * (root->num_tokens + 1));
    for (int i = 0; i < root->num_tokens; ++i)
        argv[i] = root->tokens[i].token;
    argv[root->num_tokens] = NULL;

    if ((retval = exec_builtin(root->num_tokens, argv)) == -1) {
        int status;
        pid_t pid;
        if ((pid = fork()) == -1)
            error(errno, errno, "error");
        if (pid) {
            if (waitpid(pid, &status, 0) == -1)
                error(errno, errno, "error");
        } else {
            if (execvp(argv[0], argv) == -1) {
                if (errno == ENOENT)
                    error(errno, 0, "command not found: %s", argv[0]);
                else
                    error(errno, errno, "error");
            }
        }
        retval = WEXITSTATUS(status);
    }

    free(argv);
    return retval;
}

static int exec_redir_in(struct SyntaxTree *root)
{
    int fd;
    pid_t pid;

    if ((fd = open(root->right->tokens[0].token, O_RDONLY)) == -1) {
        error(0, errno, "error");
        return errno;
    }

    if ((pid = fork()) == -1)
        error(errno, errno, "error");
    if (pid) {
        int status;
        if (waitpid(pid, &status, 0) == -1)
            error(errno, errno, "error");
        return WEXITSTATUS(status);
    } else {
        dup2(fd, 0);
        exit(exec_cmdline(root->left));
    }
}

static int exec_redir_out(struct SyntaxTree *root, bool append)
{
    int fd, flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    pid_t pid;

    if ((fd = open(root->right->tokens[0].token, flags, mode)) == -1) {
        error(0, errno, "error");
        return errno;
    }

    if ((pid = fork()) == -1)
        error(errno, errno, "error");
    if (pid) {
        int status;
        if (waitpid(pid, &status, 0) == -1)
            error(errno, errno, "error");
        return WEXITSTATUS(status);
    } else {
        dup2(fd, 1);
        exit(exec_cmdline(root->left));
    }
}

static int exec_pipe(struct SyntaxTree *root, bool err_pipe)
{
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
        error(errno, errno, "error");

    if ((pid1 = fork()) == -1)
        error(errno, errno, "error");

    if (pid1) {
        if ((pid2 = fork()) == -1)
            error(errno, errno, "error");
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
        error(errno, errno, "error");
    if (pid) {
        if (root->right)
            retval = exec_cmdline(root->right);
    } else
        exit(exec_cmdline(root->left));
    return retval;
}
