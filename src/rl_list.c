#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "rule.h"
#include "token.h"

int rl_list(struct rl_state *s)
{
    int rc;
    struct rl_ast *node;

    /* command */
    if ((rc = rl_cmd(s)) <= 0)
        return rc;
    node = calloc(1, sizeof(struct rl_ast));
    node->type = RL_LIST;
    node->child = s->ast;

    /* (';' command)* */
    struct rl_ast *child = node->child;
    while ((rc = rl_accept(s, T_SEMICOL, RL_NORULE)) == true
           && (rc = rl_cmd(s)) == true)
    {
        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (rc < 0) ? rc : 1;
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
