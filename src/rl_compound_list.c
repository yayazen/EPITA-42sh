#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_compound_list(struct rl_state *s)
{
    /* ('\n')* */
    while (rl_accept(s, T_LF, RL_NORULE) == true)
        ;

    /* and_or */
    if (rl_and_or(s) <= 0)
        return -s->err;
    struct rl_ast *node = rl_ast_new(RL_COMPOUND_LIST);
    node->child = rl_state_take_ast(s);

    struct rl_ast *seek = node->child;

    /* ((';'|'\n') ('\n')* and_or)* [(';'|'\n') ('\n')*] */
    while (rl_accept(s, T_SEMICOL, RL_NORULE) == true
           || rl_accept(s, T_LF, RL_NORULE) == true)
    {
        /* ('\n')* */
        while (rl_accept(s, T_LF, RL_NORULE) == true)
            ;

        /* and_or */
        if (rl_and_or(s) == true)
        {
            seek->sibling = rl_state_take_ast(s);
            seek = seek->sibling;
        }
        else
            break;
    }

    s->ast = node;
    return true;
}

int rl_exec_compound_list(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_COMPOUND_LIST);
    int status = 0;

    struct rl_ast *it = ast->child;
    while (it)
    {
        status = rl_exec_and_or(it);
        it = it->sibling;
    }

    return status;
}