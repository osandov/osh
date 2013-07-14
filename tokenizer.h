#ifndef STRPROC_H
#define STRPROC_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

/** A lexed token */
struct Token {
    /** Whether the token is special (e.g., an operator like `|') */
    bool special;

    union {
        /** The actual token string */
        char *token;

        /** Used internally */
        ptrdiff_t offset;
    };
};

/** Lex a string into an array of tokens */
ssize_t tokenize(struct Token **tokens, size_t *n, char *line);

/** Print a list of tokens */
void print_tokens(size_t num_tokens, struct Token *tokens);

#endif /* STRPROC_H */
