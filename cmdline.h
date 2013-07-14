#ifndef CMDLINE_H
#define CMDLINE_H

#include "parser.h"

/**
 * Execute a command line which has been parsed into a syntax tree
 * @return The exit status of the command line
 */
int exec_cmdline(struct SyntaxTree *root);

#endif /* CMDLINE_H */
