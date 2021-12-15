#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_while(struct rl_state *s)
{
    struct rl_exectree *node;

    /* While compound_list */
    if (rl_accept(s, T_WHILE, RL_NORULE) <= 0)
        return -s->err;
    s->flag |= PARSER_LINE_START;
    if (rl_compound_list(s) <= 0)
    {
        s->flag &= ~PARSER_LINE_START;
        return -s->err;
    }

    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_WHILE)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* do_group */
    if (rl_do_group(s) <= 0)
    {
        s->flag &= ~PARSER_LINE_START;
        s->node = node;
        return -s->err;
    }

    s->flag &= ~PARSER_LINE_START;
    child->sibling = s->node;
    s->node = node;

    return true;
}

int rl_exec_while(struct rl_exectree *node)
{
    assert(node && node->type == RL_WHILE);

    while (rl_exec_compound_list(node->child) == 0)
        rl_exec_compound_list(node->child->sibling);

    return 0;
}
