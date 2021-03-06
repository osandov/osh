#include <assert.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

/**
 * Construct a node of the abstract syntax tree with an array of tokens of
 * the given length
 */
static inline struct SyntaxTree *syntax_node(size_t num_tokens);

/**
 * Recursively parse a command line
 * @return Zero on success, non-zero on failure
 */
static int parse_helper(struct SyntaxTree *root);

/**
 * Split the root of a tree by trying to form two subtrees around an operator
 * for each level of precedence
 * @return Zero on success, non-zero on failure
 */
static int split_tree(struct SyntaxTree *root);

/**
 * Split a single node at a given index, creating a left and right subtree for
 * that node
 */
static void split_node(struct SyntaxTree *root, size_t i);

/**
 * Strip extra pairs of parentheses in an array of tokens of the node
 * @return Zero on success, non-zero on unmatched parentheses
 */
static int strip_parens(struct SyntaxTree *root);

/** Get the type of node corresponding to the given special token */
static inline enum NodeType get_node_type(char *token);

/** Return whether a given token is a valid special token */
static bool valid_operator(char *token);

/**
 * Return whether a syntax tree is full, i.e., all nodes that require both a
 * left and right subtree have one
 */
static bool is_full(struct SyntaxTree *root);

/**
 * Prune a syntax tree by freeing and removing any empty nodes
 */
static struct SyntaxTree *prune_tree(struct SyntaxTree *root);

/**
 * An array of null-terminated arrays containing each precedence level of
 * binary operators
 */
static const char *binary_tokens[][4] = {
    {"&", ";", "&!"}, /* Bind loosest (lowest precedence) */
    {"&&", "||"},
    {"|", "|&"},
    {"<", ">", ">>"} /* Bind tightest (highest precedence) */
};

/** The number of levels of precedence */
#define NUM_LEVELS (sizeof(binary_tokens) / sizeof(*binary_tokens))

/* See parser.h */
struct SyntaxTree *parse(size_t num_tokens, struct Token *tokens)
{
    for (int i = 0; i < num_tokens; ++i) {
        if (tokens[i].special && !valid_operator(tokens[i].token)) {
            error(0, 0, "parse error near `%s'", tokens[i].token);
            return NULL;
        }
    }

    struct SyntaxTree *root = syntax_node(num_tokens);
    memcpy(root->tokens, tokens, num_tokens * sizeof(struct Token));

    if (parse_helper(root) == -1 || !is_full(root)) {
        free_tree(root);
        return NULL;
    }
    return prune_tree(root);
}

/* See above */
static int parse_helper(struct SyntaxTree *root)
{
    if (!root->left && !root->right) {
        if (split_tree(root) == -1)
            return -1;
    } else {
        if (parse_helper(root->left) == -1 ||
            parse_helper(root->right) == -1)
            return -1;
    }
    return 0;
}


/* See above */
static inline struct SyntaxTree *syntax_node(size_t num_tokens)
{
    struct SyntaxTree *root = malloc(sizeof(struct SyntaxTree));
    if (!root)
        error(1, errno, "fatal error");
    root->tokens = malloc(sizeof(struct Token) * num_tokens);
    if (!root->tokens)
        error(1, errno, "fatal error");
    root->type = NODE_CMD;
    root->num_tokens = num_tokens;
    root->left = root->right = NULL;
    return root;
}

/* See above */
static int split_tree(struct SyntaxTree *root)
{
    ssize_t parens = 0;

    if (strip_parens(root) == -1) {
        error(0, 0, "parsing error: unmatched parentheses");
        return -1;
    }

    for (size_t level = 0; level < NUM_LEVELS; ++level) {
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
                        split_node(root, i);
                        return 0;
                    }
                }
            }
        }
    }

    return 0;
}

/* See above */
static void split_node(struct SyntaxTree *root, size_t i)
{
    root->left = syntax_node(i);
    memcpy(root->left->tokens, root->tokens,
            root->left->num_tokens * sizeof(struct Token));
    split_tree(root->left);

    root->right = syntax_node(root->num_tokens - (i + 1));
    memcpy(root->right->tokens, root->tokens + (i + 1),
            root->right->num_tokens * sizeof(struct Token));
    split_tree(root->right);

    root->tokens[0] = root->tokens[i];
    root->num_tokens = 1;
    root->type = get_node_type(root->tokens[0].token);
}

/* See above */
static int strip_parens(struct SyntaxTree *root)
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
                root->num_tokens * sizeof(struct Token));
        return strip_parens(root);
    } else
        return parens == 0 ? 0 : -1;
}

/* See above */
static inline enum NodeType get_node_type(char *token) {
    if (strcmp(token, "<") == 0)
        return NODE_REDIR_IN;
    else if (strcmp(token, ">") == 0)
        return NODE_REDIR_OUT;
    else if (strcmp(token, ">>") == 0)
        return NODE_REDIR_APPEND;
    else if (strcmp(token, "|") == 0)
        return NODE_PIPE;
    else if (strcmp(token, "|&") == 0)
        return NODE_ERR_PIPE;
    else if (strcmp(token, "&&") == 0)
        return NODE_AND;
    else if (strcmp(token, "||") == 0)
        return NODE_OR;
    else if (strcmp(token, ";") == 0)
        return NODE_SEMICOLON;
    else if (strcmp(token, "&") == 0)
        return NODE_BACKGROUND;
    else if (strcmp(token, "&!") == 0)
        return NODE_DISOWN;
    else
        assert(0);
}

/* See above */
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

/* See above */
static bool is_full(struct SyntaxTree *root)
{
    switch (root->type) {
        case NODE_CMD:
            return 1;
        case NODE_REDIR_IN:
        case NODE_REDIR_OUT:
        case NODE_REDIR_APPEND:
        case NODE_PIPE:
        case NODE_ERR_PIPE:
        case NODE_AND:
        case NODE_OR:
            if (root->left->num_tokens == 0 || root->right->num_tokens == 0) {
                error(0, 0, "parse error near `%s'", root->tokens[0].token);
                return false;
            }
            break;
        case NODE_SEMICOLON:
            break;
        case NODE_BACKGROUND:
        case NODE_DISOWN:
            if (root->left->num_tokens == 0) {
                error(0, 0, "parse error near `%s'", root->tokens[0].token);
                return false;
            }
            break;
    }
    return is_full(root->left) && is_full(root->right);
}

/** See above */
static struct SyntaxTree *prune_tree(struct SyntaxTree *root)
{
    if (root) {
        if (root->num_tokens == 0) {
            free_tree(root);
            root = NULL;
        } else {
            root->left = prune_tree(root->left);
            root->right = prune_tree(root->right);
        }
    }
    return root;
}

/** See parser.h */
void free_tree(struct SyntaxTree *root)
{
    if (root) {
        free(root->tokens);
        free_tree(root->left);
        free_tree(root->right);
        free(root);
    }
}

/** See parser.h */
void print_tree(struct SyntaxTree *root)
{
    if (root) {
        print_tokens(root->num_tokens, root->tokens);
        print_tree(root->left);
        print_tree(root->right);
    }
}
