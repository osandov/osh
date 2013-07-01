#ifndef STRPROC_H
#define STRPROC_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

struct Token {
    bool special;
    union {
        char *token;
        ptrdiff_t offset;
    };
};

ssize_t tokenize(struct Token **tokens, size_t *n, char *line);
void print_tokens(size_t num_tokens, struct Token *tokens);

#endif /* STRPROC_H */
