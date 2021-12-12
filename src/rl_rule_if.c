#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_rule_if(struct rl_state *s)
{
    struct rl_ast *node;

    /* if */
    if (rl_accept(s, T_IF, RL_NORULE) <= 0)
        return -s->err;

    /* compound_list */
    if (rl_compound_list(s) <= 0)
        return -s->err;

    /* alloc node */
    assert(node = calloc(1, sizeof(struct rl_ast)));
    node->type = RL_IF;
    node->child = s->ast;
    s->ast = NULL;

    /* then */
    if (rl_accept(s, T_THEN, RL_NORULE) <= 0)
    {
        rl_ast_free(node);
        return -s->err;
    }

    /* compound_list */
    if (rl_compound_list(s) <= 0)
    {
        rl_ast_free(node);
        return -s->err;
    }

    node->child->sibling = s->ast;
    node->child->sibling->sibling = NULL;
    s->ast = NULL;

    /* fi */
    if (rl_accept(s, T_FI, RL_NORULE) <= 0)
    {
        rl_ast_free(node);
        return -s->err;
    }

    s->ast = node;

    return true;
}

int rl_exec_rule_if(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_IF);

    int status = rl_exec_compound_list(ast->child);

    // If the `if` condition is met
    if (status == 0)
    {
        status = rl_exec_compound_list(ast->child->sibling);
    }

    return status;
}