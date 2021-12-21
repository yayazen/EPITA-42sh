#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_until(struct rl_state *s)
{
    struct rl_exectree *node;

    /* Until compound_list */
    if (rl_accept(s, T_UNTIL) <= 0 || rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_UNTIL)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* do_group */
    if (rl_do_group(s) <= 0)
    {
        s->node = node;
        return -s->err;
    }
    child->sibling = s->node;

    s->node = node;

    return true;
}

int rl_exec_until(struct rl_exectree *node, const struct ctx *ctx)
{
    assert(node && node->type == RL_UNTIL);

    while (rl_exec_compound_list(node->child, ctx) != 0)
        rl_exec_compound_list(node->child->sibling, ctx);

    return 0;
}