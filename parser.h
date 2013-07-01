#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

struct syntax_tree {
    struct token *tokens;
    size_t num_tokens;
    struct syntax_tree *left, *right;
};

struct syntax_tree *parse(size_t token_count, struct token *tokens);
void free_tree(struct syntax_tree *root);
void print_tree(struct syntax_tree *root);

#endif /* PARSER_H */
