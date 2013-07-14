#ifndef BUILTIN_H
#define BUILTIN_H

/**
 * Execute a built-in shell command with the given command line arguments
 * @return The return status of the command, or -1 if there was an error (e.g.,
 * the command was not found
 */
int exec_builtin(int argc, char **argv);

#endif /* BUILTIN_H */
