#include <ctype.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.h"

#define INITIAL_BUFFER_SIZE 256

static const char *special_strings[] = {
    "&&", "||", ">&", "<<", ">>", "&!", "|&"
};

static inline bool isspecial(char c)
{
    return c == '&' || c == '|' ||
           c == '!' || c == '#' ||
           c == '<' || c == '>' ||
           c == '(' || c == ')' ||
           c == ';';
}

static inline bool need_split(char curr, char prev)
{
    if (isspace(prev))
        return false;
    else {
        for (int i = 0; i < sizeof(special_strings) / sizeof(*special_strings);
                ++i) {
            if (prev == special_strings[i][0] && curr == special_strings[i][1])
                return false;
        }
    }
    return true;
}

static void add_token(struct Token **tokens, size_t *n, size_t i,
                      ptrdiff_t offset, bool special)
{
    if (i >= *n) {
        struct Token *new_tokens;
        *n = 2 * *n + 1;
        if (!(new_tokens = realloc(*tokens, *n * sizeof((*tokens)[0]))))
            error(1, errno, "fatal error");
        *tokens = new_tokens;
    }
    (*tokens)[i].offset = offset;
    (*tokens)[i].special = special;
}

static void write_char(char **buffer, size_t *n, char **p, char c)
{
    ptrdiff_t d = *p - *buffer;
    if (d >= *n) {
        char *new_buffer;
        *n = 2 * *n + 1;
        if (!(new_buffer = realloc(*buffer, *n * sizeof(char))))
            error(1, errno, "fatal error");
        *buffer = new_buffer;
        *p = new_buffer + d;
    }
    *((*p)++) = c;
}

#define ENTER_TOKEN(a)                                 \
    do {                                               \
        add_token(tokens, n, i++, head - buffer, (a)); \
        in_token = true;                               \
    } while(0)

#define LEAVE_TOKEN()                                  \
    do {                                               \
        write_char(&buffer, &buffer_len, &head, '\0'); \
        in_token = false;                              \
        number = true;                                 \
    } while(0)

#define WRITE_CHAR(c)                                 \
    do {                                              \
        write_char(&buffer, &buffer_len, &head, (c)); \
        if (!isdigit(c))                              \
            number = false;                           \
    } while (0);

ssize_t tokenize(struct Token **tokens, size_t *n, char *line)
{
    static char *buffer = NULL;
    static size_t buffer_len = INITIAL_BUFFER_SIZE;

    if (!buffer) {
        buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
        if (!buffer)
            error(1, errno, "fatal error");
    }

    char *head = buffer;
    char prev = ' ', quote = '\0';
    ssize_t i = 0;
    bool in_token = false, escape = false, number = true;

    for (char *p = line; *p; ++p) {
        char c = *p;
        if (quote) {
            if (!in_token)
                ENTER_TOKEN(false);
            if (c == quote)
                quote = '\0';
            else
                WRITE_CHAR(c);
        } else if (escape) {
            if (!in_token)
                ENTER_TOKEN(false);
            WRITE_CHAR(c);
            escape = false;
        } else {
            if (isspecial(prev) && !isspecial(c))
                LEAVE_TOKEN();
            if (isspace(c)) {
                if (!isspace(prev) && !isspecial(prev))
                    LEAVE_TOKEN();
            } else {
                if (isspecial(c)) {
                    if (number && c == '>') {
                        if (in_token)
                            (*tokens)[i - 1].special = true;
                    } else if (need_split(c, prev))
                        LEAVE_TOKEN();
                    if (!in_token)
                        ENTER_TOKEN(true);
                }
                if (c == '\'' || c == '"')
                    quote = c;
                else {
                    if (!in_token)
                        ENTER_TOKEN(false);
                    escape = c == '\\';
                    if (!escape)
                        WRITE_CHAR(c);
                }
            }
        }
        prev = c;
    }

    if (quote) {
        error(0, 0, "error: Unclosed quote");
        return -1;
    }

    for (ssize_t j = 0; j < i; ++j)
        (*tokens)[j].token = buffer + (*tokens)[j].offset;

    return i;
}

void print_tokens(size_t num_tokens, struct Token *tokens)
{
    printf("[");
    if (num_tokens) {
        printf("`%s'", tokens[0].token);
        if (tokens[0].special)
            printf("*");
        for (int i = 1; i < num_tokens; ++i) {
            printf(", `%s'", tokens[i].token);
            if (tokens[i].special)
                printf("*");
        }
    }
    printf("]\n");
}
