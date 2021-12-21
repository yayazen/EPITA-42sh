#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_compound_list(struct rl_state *s)
{
    struct rl_exectree *node;

    /* ('\n')* */
    while (rl_accept(s, T_LF) == true)
        ;

    /* and_or */
    if (rl_and_or(s) <= 0)
        return -s->err;
    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_COMPOUND_LIST)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* ((';'|'\n') ('\n')* and_or)* [(';'|'\n') ('\n')*] */
    while (rl_accept(s, T_SEMICOL) == true || rl_accept(s, T_LF) == true)
    {
        while (rl_accept(s, T_LF) == true)
            ;

        /* and_or */
        if (rl_and_or(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_compound_list(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_COMPOUND_LIST);

    int status;
    node = node->child;
    do
    {
        status = rl_exec_and_or(ctx, node);

    } while ((node = node->sibling));

    return status;
}
