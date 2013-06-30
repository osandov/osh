#ifndef STRPROC_H
#define STRPROC_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct token {
    bool special;
    union {
        char *token;
        ptrdiff_t offset;
    };
};

ssize_t tokenize(struct token **tokens, size_t *n, char *line);

#endif /* STRPROC_H */
