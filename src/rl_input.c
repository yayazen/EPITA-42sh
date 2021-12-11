#include <assert.h>

#include "debug.h"
#include "rule.h"
#include "token.h"

int rl_input(struct rl_state *s)
{
    int rc;

    struct rl_ast *node = calloc(1, sizeof(struct rl_ast));
    node->type = RL_INPUT;

    /* if only '\n' | EOF */
    if ((rc = rl_accept(s, T_LF, RL_NORULE)) == true
        || (rc = rl_accept(s, T_EOF, RL_NORULE)) == true)
    {
        s->ast = node;
        return true;
    }

    /* list */
    if ((rc = rl_list(s)) == true)
    {
        node->child = s->ast;
    }

    /* mandatory '\n' | EOF */
    if ((rc = rl_accept(s, T_LF, RL_NORULE)) != true
        && (rc = rl_accept(s, T_EOF, RL_NORULE)) != true)
    {
        rl_ast_free(node);
        rc = 0;
        s->ast = NULL;
    }

    else
    {
        s->ast = node;
    }

    return (rc <= 0) ? rc : true;
}

int rl_exec_input(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_INPUT);

    if (ast->child != NULL)
        return rl_exec_list(ast->child);

    return 0;
}