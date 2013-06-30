#include <ctype.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "tokenizer.h"

#define INITIAL_BUFFER_SIZE 256

/* #ifdef DEBUG_TOKENS */

char escape_sequence(char c)
{
    switch (c) {
        case 'a':
            return '\a';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        /* case '0': */
            /* return '\0'; */
        default:
            return c;
    }
}

static inline int isspecial(int c)
{
    return c == '&' || c == '|' || c == '!' || c == '#';
}

static inline bool need_split(char curr, char prev)
{
    return !isspace(prev) &&
        (curr == '#' || (curr != prev && !(prev == '&' && curr == '!')));
}

static void add_token(struct token **tokens, size_t *n, size_t i,
                      ptrdiff_t offset, bool special)
{
    if (i >= *n) {
        struct token *new_tokens;
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

ssize_t tokenize(struct token **tokens, size_t *n, char *line)
{
    static char *buffer = NULL;
    static size_t buffer_len = INITIAL_BUFFER_SIZE;

    if (!buffer) {
        buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
        if (!buffer)
            error(1, errno, "fatal error");
    }

    char *head = buffer;
    ptrdiff_t pos = -1;
    ssize_t i = 0;

    char prev = ' ', quote = '\0';
    bool escape = false;

    for (char *p = line; *p; ++p) {
        char c = *p;
        if (quote) {
            if (pos == -1)
                pos = head - buffer;
            if (c == quote)
                quote = '\0';
            else {
                if (escape) {
                    char e = escape_sequence(c);
                    if (e)
                        write_char(&buffer, &buffer_len, &head, e);
                } else {
                    escape = c == '\\';
                    if (!escape)
                        write_char(&buffer, &buffer_len, &head, c);
                }
            }
        } else {
            if (isspecial(prev) && !isspecial(c)) {
                write_char(&buffer, &buffer_len, &head, '\0');
                add_token(tokens, n, i++, pos, true);
                pos = -1;
            }
            if (isspace(c)) {
                if (!isspace(prev) && !isspecial(prev)) {
                    write_char(&buffer, &buffer_len, &head, '\0');
                    add_token(tokens, n, i++, pos, false);
                    pos = -1;
                }
            } else {
                if (isspecial(c) && need_split(c, prev)) {
                    write_char(&buffer, &buffer_len, &head, '\0');
                    add_token(tokens, n, i++, pos, false);
                    pos = -1;
                }
                if (c == '\'' || c == '"')
                    quote = c;
                else {
                    if (pos == -1)
                        pos = head - buffer;
                    write_char(&buffer, &buffer_len, &head, c);
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


#ifdef DEBUG_TOKENS
    for (int j = 0; j < i; ++j) {
        printf("'%s'", (*tokens)[j].token);
        if ((*tokens)[j].special)
            printf(" (*)\n");
        else
            printf("\n");
    }
    bool temp = false;
    *head = '\0';
    for (head = line_buffer; ; ++head) {
        if (*head) {
            temp = false;
            printf("%c", *head);
        } else if (temp)
            break;
        else {
            temp = true;
            printf("\n");
        }
    }
#endif

    return i;
}
