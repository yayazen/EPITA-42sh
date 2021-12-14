#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_while(struct rl_state *s)
{
    struct rl_ast *node;

    /* While compound_list */
    if (rl_accept(s, T_WHILE, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_WHILE)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* do_group */
    if (rl_do_group(s) <= 0)
    {
        s->ast = node;
        return -s->err;
    }
    child->sibling = s->ast;
    child = child->sibling;

    s->ast = node;

    return true;
}

int rl_exec_while(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_WHILE);

    while (rl_exec_compound_list(ast->child) == 0)
        rl_exec_compound_list(ast->child->sibling);

    return 0;
}