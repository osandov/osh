#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

enum NodeType {
    NODE_CMD,
    NODE_REDIR_IN,
    NODE_REDIR_OUT,
    NODE_REDIR_APPEND,
    NODE_PIPE,
    NODE_ERR_PIPE,
    NODE_AND,
    NODE_OR,
    NODE_SEMICOLON,
    NODE_BACKGROUND,
    NODE_DISOWN
};

struct SyntaxTree {
    enum NodeType type;
    struct Token *tokens;
    size_t num_tokens;
    struct SyntaxTree *left, *right;
};

struct SyntaxTree *parse(size_t token_count, struct Token *tokens);
void free_tree(struct SyntaxTree *root);
void print_tree(struct SyntaxTree *root);

#endif /* PARSER_H */
