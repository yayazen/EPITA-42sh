#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_if_clause(struct rl_state *s)
{
    struct rl_ast *node;

    /* if compound_list */
    if (rl_accept(s, T_IF, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_IF)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* then compound_list */
    if (rl_expect(s, T_THEN, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
    {
        s->ast = node;
        return -s->err;
    }
    child->sibling = s->ast;
    child = child->sibling;

    /* [else_clause] */
    if (rl_else_clause(s) == true)
        child->sibling = s->ast;

    /* fi */
    if (rl_expect(s, T_FI, RL_NORULE) <= 0)
    {
        s->ast = node;
        return -s->err;
    }
    s->ast = node;

    return true;
}

int rl_exec_if_clause(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_IF);

    ast = ast->child;
    // If the `if` condition is met
    if (rl_exec_compound_list(ast) == 0)
    {
        return rl_exec_compound_list(ast->sibling);
    }
    // Otherwise, run `else clause` (if available)
    else if ((ast = ast->sibling)->sibling)
    {
        return rl_exec_else_clause(ast->sibling);
    }

    return 0;
}
