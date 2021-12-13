#include <assert.h>

#include "rule.h"
#include "token.h"

int rl_compound_list(struct rl_state *s)
{
    struct rl_ast *node;

    /* ('\n')* */
    while (rl_accept(s, T_LF, RL_NORULE) == true)
        ;

    /* and_or */
    if (rl_and_or(s) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_COMPOUND_LIST)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* ((';'|'\n') ('\n')* and_or)* [(';'|'\n') ('\n')*] */
    while (rl_accept(s, T_SEMICOL, RL_NORULE) == true
           || rl_accept(s, T_LF, RL_NORULE) == true)
    {
        while (rl_accept(s, T_LF, RL_NORULE) == true)
            ;

        /* and_or */
        if (rl_and_or(s) <= 0)
            break;

        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_compound_list(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_COMPOUND_LIST);

    int status;
    ast = ast->child;
    do
    {
        status = rl_exec_and_or(ast);

    } while ((ast = ast->sibling));

    return status;
}
