#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_until(struct rl_state *s)
{
    struct rl_ast *node;

    /* Until compound_list */
    if (rl_accept(s, T_UNTIL, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_UNTIL)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* do_group */
    if (rl_do_group(s) <= 0)
    {
        s->ast = node;
        return -s->err;
    }
    child->sibling = s->ast;

    s->ast = node;

    return true;
}

int rl_exec_until(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_UNTIL);

    while (rl_exec_compound_list(ast->child) != 0)
        rl_exec_compound_list(ast->child->sibling);

    return 0;
}