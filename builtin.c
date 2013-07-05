#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef int(*builtin_function)(int, char**);

static int builtin_cd(int argc, char **argv);
static int builtin_exit(int argc, char **argv);

struct builtin_entry {
    const char *name;
    builtin_function func;
};

static struct builtin_entry builtins[] = {
    {"cd", builtin_cd},
    {"exit", builtin_exit},
};

int exec_builtin(int argc, char **argv)
{
    for (int i = 0; i < sizeof(builtins) / sizeof(*builtins); ++i) {
        if (strcmp(builtins[i].name, argv[0]) == 0)
            return builtins[i].func(argc, argv);
    }
    return -1;
}

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

static int builtin_exit(int argc, char **argv)
{
    int status = 0;
    if (argc > 1)
        status = atoi(argv[1]) % 256;
    exit(status);
}
