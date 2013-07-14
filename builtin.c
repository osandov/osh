#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * A built-in command taking an arbitrary number of arguments
 * @return The exit status of the command
 */
typedef int(*builtin_function)(int, char**);

/**
 * Change the current working directory of the shell. If no arguments are
 * given, change to the users home directory based on the HOME environment
 * variable. A dash ('-') can be given to change to the previous working
 * directory
 */
static int builtin_cd(int argc, char **argv);

/**
 * Exit the shell with an exit status given by an optional first argument
 * (defaults to 0)
 */
static int builtin_exit(int argc, char **argv);

/** An entry in the table of built-ins */
struct builtin_entry {
    /** The command string */
    const char *name;

    /** The function to be executed for that command */
    builtin_function func;
};

/** The table of built-in command */
static struct builtin_entry builtins[] = {
    {"cd", builtin_cd},
    {"exit", builtin_exit},
};

/* See builtin.h */
int exec_builtin(int argc, char **argv)
{
    for (int i = 0; i < sizeof(builtins) / sizeof(*builtins); ++i) {
        if (strcmp(builtins[i].name, argv[0]) == 0)
            return builtins[i].func(argc, argv);
    }
    return -1;
}

/* See above */
static int builtin_cd(int argc, char **argv)
{
    static char *prev_path = NULL;
    char *cwd = getcwd(NULL, 0), *path = argv[1];

    if (!cwd) {
        error(0, errno, "error");
        return 1;
    }

    if (!prev_path)
        prev_path = cwd;

    if (!argv[1])
        path = getenv("HOME");
    else if (strcmp(argv[1], "-") == 0)
        path = prev_path;

    if (chdir(path) == -1) {
        error(0, errno, "error");
        return 1;
    }

    prev_path = cwd;
    return 0;
}

/* See above */
static int builtin_exit(int argc, char **argv)
{
    int status = 0;
    if (argc > 1)
        status = atoi(argv[1]) % 256;
    exit(status);
}
