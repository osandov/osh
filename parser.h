#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

enum NodeType {
    NODE_PIPELINE,
    NODE_AND,
    NODE_OR,
    NODE_BACKGROUND,
    NODE_SEMICOLON,
    NODE_DISOWN
};

struct SyntaxTree {
    enum NodeType type;
    struct {
        struct Token *tokens;
        size_t num_tokens;
    };
    struct SyntaxTree *left, *right;
};

struct SyntaxTree *parse(size_t token_count, struct Token *tokens);
void free_tree(struct SyntaxTree *root);
void print_tree(struct SyntaxTree *root);

#endif /* PARSER_H */
