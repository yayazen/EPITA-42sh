#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_list(struct rl_state *s)
{
    struct rl_ast *node;

    /* and_or */
    if (rl_and_or(s) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_LIST)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* ((';'|'&') and_or)* [';'|'&'] */
    while (rl_accept(s, T_SEMICOL, RL_NORULE) == true
           || rl_accept(s, T_AND, RL_NORULE) == true)
    {
        if (rl_and_or(s) <= 0)
            break;
        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_list(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_LIST);

    int status;
    ast = ast->child;
    do
    {
        status = rl_exec_and_or(ast);
    } while ((ast = ast->sibling));

    return status;
}
