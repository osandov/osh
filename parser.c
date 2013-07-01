#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static int split_tree(struct syntax_tree *root, size_t level);
static void split_node(struct syntax_tree *root, size_t i, size_t level);

static const char *binary_tokens[][4] = {
    {"&", ";", "&!"},
    {"&&", "||"},
};

#define NUM_LEVELS (sizeof(binary_tokens) / sizeof(*binary_tokens))

static inline struct syntax_tree *syntax_node(size_t num_tokens)
{
    struct syntax_tree *root = malloc(sizeof(struct syntax_tree));
    if (!root)
        error(1, errno, "fatal error");
    root->tokens = malloc(sizeof(struct token) * num_tokens);
    if (!root->tokens)
        error(1, errno, "fatal error");
    root->num_tokens = num_tokens;
    root->left = root->right = NULL;
    return root;
}

static int strip_parens(struct syntax_tree *root)
{
    if (root->num_tokens == 0)
        return 0;

    ssize_t parens = 0;
    bool strip = root->tokens[0].special &&
                 strcmp(root->tokens[0].token, "(") == 0 &&
                 root->tokens[root->num_tokens - 1].special &&
                 strcmp(root->tokens[root->num_tokens - 1].token, ")") == 0;

    for (size_t i = 0; i < root->num_tokens; ++i) {
        if (root->tokens[i].special) {
            char *token = root->tokens[i].token;
            if (strcmp(token, "(") == 0)
                ++parens;
            else if (strcmp(token, ")") == 0) {
                --parens;
                if (parens == 0 && i != root->num_tokens - 1)
                    strip = false;
                else if (parens < 0)
                    return -1;
            }
        }
    }
    if (strip) {
        root->num_tokens -= 2;
        memmove(root->tokens, root->tokens + 1,
                root->num_tokens * sizeof(struct token));
        return strip_parens(root);
    } else
        return parens == 0 ? 0 : -1;
}

static int split_tree(struct syntax_tree *root, size_t level)
{
    ssize_t parens = 0;

    if (strip_parens(root) == -1) {
        error(0, 0, "parsing error: unmatched parentheses");
        return -1;
    }

    for (ssize_t i = root->num_tokens - 1; i >= 0; --i) {
        const char **operators = binary_tokens[level];
        for (size_t j = 0; operators[j]; ++j) {
            if (root->tokens[i].special) {
                char *token = root->tokens[i].token;
                if (strcmp(token, ")") == 0)
                    ++parens;
                else if (strcmp(token, "(") == 0)
                    --parens;
                else if (!parens && strcmp(token, operators[j]) == 0) {
                    split_node(root, i, level);
                    return 0;
                }
            }
        }
    }

    return 0;
}

static void split_node(struct syntax_tree *root, size_t i, size_t level)
{
    root->left = syntax_node(i);
    memcpy(root->left->tokens, root->tokens,
            root->left->num_tokens * sizeof(struct token));
    split_tree(root->left, level);

    root->right = syntax_node(root->num_tokens - (i + 1));
    memcpy(root->right->tokens, root->tokens + (i + 1),
            root->right->num_tokens * sizeof(struct token));
    split_tree(root->right, level);
    root->tokens[0] = root->tokens[i];
    root->num_tokens = 1;
}

static int parse_level(struct syntax_tree *root, size_t level)
{
    if (!root->left && !root->right) {
        if (split_tree(root, level) == -1)
            return -1;
    } else {
        if (parse_level(root->left, level) == -1 ||
            parse_level(root->right, level) == -1)
            return -1;
    }
    return 0;
}

static bool valid_operator(char *token)
{
    for (size_t i = 0; i < NUM_LEVELS; ++i) {
        const char **operators = binary_tokens[i];
        for (size_t j = 0; operators[j]; ++j) {
            if (strcmp(token, operators[j]) == 0)
                return true;
        }
        if (strcmp(token, "(") == 0 || strcmp(token, ")") == 0)
            return true;
    }
    return false;
}

struct syntax_tree *parse(size_t num_tokens, struct token *tokens)
{
    for (int i = 0; i < num_tokens; ++i) {
        if (tokens[i].special && !valid_operator(tokens[i].token)) {
            error(0, 0, "parse error near `%s'", tokens[i].token);
            return NULL;
        }
    }

    struct syntax_tree *root = syntax_node(num_tokens);
    memcpy(root->tokens, tokens, num_tokens * sizeof(struct token));
    for (int i = 0; i < NUM_LEVELS; ++i) {
        if (parse_level(root, i) == -1)
            return NULL;
    }
    return root;
}

void free_tree(struct syntax_tree *root)
{
    if (root) {
        free(root->tokens);
        free_tree(root->left);
        free_tree(root->right);
        free(root);
    }
}

void print_tree(struct syntax_tree *root)
{
    if (root) {
        print_tokens(root->num_tokens, root->tokens);
        print_tree(root->left);
        print_tree(root->right);
    }
}
