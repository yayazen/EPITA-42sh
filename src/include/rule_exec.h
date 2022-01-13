#pragma once

#include <sys/types.h>
#include <unistd.h>

#include "rule.h"
#include "symtab.h"

// Hack for test units, which are written in C++, which
// requires explicit cast when returning from malloc functions
#ifdef __cplusplus
#    define CAST_AST(a) (struct rl_exectree *)(a)
#else
#    define CAST_AST(a) a
#endif

struct attr_cmd
{
    int status;
    /* store node exec pid */
    pid_t pid;
    /* file descriptors to replace STDIN, STDOUT and STDERR */
    int fd[3];
    /* execution arguments */
    char **argv;
};

struct attr_pipeline
{
    /* AND_OR operator */
    int and_or_op;

    int negate;
    int fd[2];
};

struct attr_redir
{
    /* redirection token */
    int token;
    /* left side ionumber */
    int ionumber;
    /* right side file */
    char *file;
};

union attr
{
    char *word;
    struct attr_cmd cmd;
    struct attr_redir redir;
    struct attr_pipeline pipe;
};

struct rl_exectree
{
    /* the node rule type */
    int type;
    /* attributes for execution */
    union attr attr;
    /* left-child */
    struct rl_exectree *child;
    /* right-sibling */
    struct rl_exectree *sibling;
};

#define __rl_exectree_has_word(type)                                           \
    (type == RL_WORD || type == RL_ASSIGN_WORD || type == RL_FOR               \
     || type == RL_CASE)

/**
 * \brief create a new exec node with a predefined type.
 */
static inline struct rl_exectree *rl_exectree_new(int rltype)
{
    struct rl_exectree *node = CAST_AST(calloc(1, sizeof(struct rl_exectree)));
    if (!node)
        return NULL;
    node->type = rltype;
    if (rltype == RL_SIMPLE_CMD || rltype == RL_SHELL_CMD)
    {
        node->attr.cmd.pid = -1;
        node->attr.cmd.fd[0] = STDIN_FILENO;
        node->attr.cmd.fd[1] = STDOUT_FILENO;
        node->attr.cmd.fd[2] = STDERR_FILENO;
    }
    else if (rltype == RL_REDIRECTION)
    {
        node->attr.redir.ionumber = STDOUT_FILENO;
        node->attr.redir.token = -1;
        node->attr.redir.file = NULL;
    }
    else
    {
        node->attr.word = NULL;
    }

    return node;
}

/**
 * @brief Clone an exec node recursively
 *
 * @param src The source node to clone
 * @return The cloned node
 */
static inline struct rl_exectree *
rl_exectree_clone(const struct rl_exectree *src)
{
    if (!src)
        return NULL;

    struct rl_exectree *clone = rl_exectree_new(src->type);

    memcpy(&clone->attr, &src->attr, sizeof(union attr));

    if (__rl_exectree_has_word(src->type) && src->attr.word)
        clone->attr.word = strdup(src->attr.word);
    else if (src->type == RL_REDIRECTION && src->attr.redir.file)
        clone->attr.redir.file = strdup(src->attr.redir.file);

    clone->child = rl_exectree_clone(src->child);
    clone->sibling = rl_exectree_clone(src->sibling);

    return clone;
}

/**
 * \brief free the AST recursively.
 */
static inline void rl_exectree_free(struct rl_exectree *node)
{
    if (!node)
        return;
    rl_exectree_free(node->child);
    rl_exectree_free(node->sibling);

    if (__rl_exectree_has_word(node->type) && node->attr.word)
        free(node->attr.word);
    else if (node->type == RL_REDIRECTION && node->attr.redir.file)
        free(node->attr.redir.file);
    free(node);
}

#undef __rl_exectree_has_word
