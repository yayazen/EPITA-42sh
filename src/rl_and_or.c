#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_and_or(struct rl_state *s)
{
    struct rl_exectree *node;

    /* pipeline */
    if (rl_pipeline(s) <= 0)
        return -s->err;
    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_AND_OR)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* (('&&'|'||') ('\n')* pipeline)* */
    while (rl_accept(s, T_AND_IF) == true || rl_accept(s, T_OR_IF) == true)
    {
        child->attr.pipe.and_or_op = s->token;

        while (rl_accept(s, T_LF) == true)
            ;

        if (rl_pipeline(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_and_or(struct rl_exectree *node, const struct ctx *ctx)
{
    assert(node && node->child && node->type == RL_AND_OR);

    int status;
    int exec_next = 1;
    node = node->child;
    do
    {
        if (exec_next)
            status = rl_exec_pipeline(node, ctx);

        exec_next = (node->attr.pipe.and_or_op == T_AND_IF && status == 0)
            || (node->attr.pipe.and_or_op == T_OR_IF && status != 0);

    } while ((node = node->sibling));

    return status;
}
