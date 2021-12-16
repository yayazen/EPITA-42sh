#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_if_clause(struct rl_state *s)
{
    struct rl_exectree *node;

    /* if compound_list */
    if (rl_accept(s, T_IF) <= 0)
        return -s->err;
    s->flag |= PARSER_LINE_START;
    if (rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_IF)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* then compound_list */
    if (rl_expect(s, T_THEN) <= 0 || rl_compound_list(s) <= 0)
    {
        s->node = node;
        return -s->err;
    }
    child->sibling = s->node;
    child = child->sibling;

    /* [else_clause] */
    if (rl_else_clause(s) == true)
        child->sibling = s->node;

    /* fi */
    if (rl_expect(s, T_FI) <= 0)
    {
        s->node = node;
        return -s->err;
    }
    s->flag &= ~PARSER_LINE_START;
    s->node = node;

    return true;
}

int rl_exec_if_clause(struct rl_exectree *node)
{
    assert(node && node->type == RL_IF);

    node = node->child;
    // If the `if` condition is met
    if (rl_exec_compound_list(node) == 0)
    {
        return rl_exec_compound_list(node->sibling);
    }
    // Otherwise, run `else clause` (if available)
    else if ((node = node->sibling)->sibling)
    {
        return rl_exec_else_clause(node->sibling);
    }

    return 0;
}
