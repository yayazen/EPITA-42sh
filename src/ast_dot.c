/**
 * \file ast_dot.c
 * \brief AST Dot debug printer implementation
 */

#include "ast_dot.h"

#include <assert.h>
#include <stdio.h>

#include "rule.h"
#include "token.h"

/** \brief recursive ast dot print function*/
static int __aux_ast_dot(struct rl_exectree *node, int p, int c)
{
    if (!node)
        return c;
    c++;

    fprintf(stdout, "%d", c);
    if (node->type == RL_WORD)
    {
        fprintf(stdout, " [label= \"%s (%d)\"]\n", node->attr.word, c);
    }
    else if (node->type == RL_ASSIGN_WORD)
    {
        fprintf(stdout, " [label= \"%s (%d)\"]\n", node->attr.word, c);
    }
    else if (node->type == RL_REDIRECTION)
    {
        struct attr_redir *redir = &node->attr.redir;
        fprintf(stdout, " [label= \"%d %s %s (%d)\"]\n", redir->ionumber,
                TOKEN_STR(redir->token), redir->file, c);
    }
    else
    {
#define RULE(Rl, Str)                                                          \
    if (Rl == node->type)                                                      \
        fprintf(stdout, " [label= \"%s (%d)\"]\n", Str, c);
#include "rule.def"
#undef RULE
    }

    fprintf(stdout, "%d -- %d\n", p, c);
    c = __aux_ast_dot(node->child, c, c);
    c = __aux_ast_dot(node->sibling, p, c);
    return c;
}

void ast_dot_print(struct rl_exectree *node)
{
    if (!node)
        return;

    fprintf(stdout, "graph {\n");
    fprintf(stdout, "0 [label= \"input\"]\n");
    __aux_ast_dot(node->child, 0, 1);
    fprintf(stdout, "} \n");
}
