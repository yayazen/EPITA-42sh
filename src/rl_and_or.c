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
    while (rl_accept(s, T_AND_IF, RL_NORULE) == true
           || rl_accept(s, T_OR_IF, RL_NORULE) == true)
    {
        while (rl_accept(s, T_LF, RL_NORULE) == true)
            ;

        if (rl_pipeline(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_and_or(struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_AND_OR);

    int status;
    node =node->child;
    do
    {
        status = rl_exec_pipeline(node);

    } while ((node = node->sibling));

    return status;
}
