#include "rule.h"
#include "token.h"

int rl_and_or(struct rl_state *s)
{
    struct rl_ast *node;

    /* pipeline */
    if (rl_pipeline(s) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_AND_OR)))
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

        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_and_or(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_AND_OR);

    int status;
    ast = ast->child;
    do
    {
        status = rl_exec_pipeline(ast);

    } while ((ast = ast->sibling));

    return status;
}
