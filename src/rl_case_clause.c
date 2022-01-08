#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_case_clause(struct rl_state *s)
{
    struct rl_exectree *node;

    /* case_item */
    if (rl_case_item(s) <= 0)
        return -s->err;

    assert(node = rl_exectree_new(RL_CASE_CLAUSE));
    node->child = s->node;

    struct rl_exectree *child = node->child;

    /* [;;] */
    while (rl_accept(s, T_DSEMICOL) == true)
    {
        /* ('\n')* */
        while (rl_accept(s, T_LF) == true)
            ;

        /* case_item */
        if (rl_case_item(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}