#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "rule.h"
#include "token.h"

int rl_list(struct rl_state *s)
{
    struct rl_ast *node;

    /* command */
    if (rl_cmd(s) <= 0)
        return -s->err;
    if (!(node = calloc(1, sizeof(struct rl_ast))))
        return -(s->err = UNKNOWN_ERROR);
    node->type = RL_LIST;
    node->child = s->ast;

    /* (';' command)* */
    struct rl_ast *child = node->child;
    while (rl_accept(s, T_SEMICOL, RL_NORULE) == true && rl_cmd(s) == true)
    {
        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : 1;
}

int rl_exec_list(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_LIST);

    int rc;
    ast = ast->child;
    while (ast)
    {
        rc = rl_exec_cmd(ast);
        ast = ast->sibling;
    }

    return rc;
}
