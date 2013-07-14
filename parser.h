#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

/** Constants for the types of (special) nodes */
enum NodeType {
    NODE_CMD, /**< A normal node which is to be executed literally */
    NODE_REDIR_IN, /**< An input redirection node */
    NODE_REDIR_OUT, /**< An output redirection node */
    NODE_REDIR_APPEND, /**< An appending output redirection node */
    NODE_PIPE, /**< A pipe node */
    NODE_ERR_PIPE, /**< A standard-error pipe node */
    NODE_AND, /**< A logical-AND node */
    NODE_OR, /**< A logical-OR node */
    NODE_SEMICOLON, /**< A sequential list separator node */
    NODE_BACKGROUND, /**< A backgrounding (non-blocking) list separator node */
    NODE_DISOWN /** A backgrounding and disowning list separator node */
};

/** An abstract syntax tree resulting from parsing a command line */
struct SyntaxTree {
    /** The type of node (e.g., a literal command or a pipe) */
    enum NodeType type;

    /**
     * The tokens in the node (either just the operator for special nodes or
     * the command and arguments for command nodes)
     */
    struct Token *tokens;

    /** The number of tokens in the nodes */
    size_t num_tokens;

    /** The left and right subtrees */
    struct SyntaxTree *left, *right;
};

/** Parse an array of tokens into an abstract syntax tree */
struct SyntaxTree *parse(size_t token_count, struct Token *tokens);

/** Free an abstract syntax tree */
void free_tree(struct SyntaxTree *root);

/** Print an abstract syntax tree by a pre-order traversal */
void print_tree(struct SyntaxTree *root);

#endif /* PARSER_H */
