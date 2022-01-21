#include <assert.h>

#include "constants.h"
#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    s->flag |= LEX_CMDSTART;
    s->node = NULL;
    int res = rl_shell_cmd(s);

    if (res == false)
        res = rl_fundec(s);

    if (res == false)
        res = rl_simple_cmd(s);

    if (res <= 0)
    {
        rl_exectree_free(s->node);
        s->node = NULL;
    }
    return res;
}

int rl_exec_cmd(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node);

    if (node->type == RL_SIMPLE_CMD)
        return rl_exec_simple_cmd(ctx, node);
    else if (node->type == RL_SHELL_CMD)
        return rl_exec_shell_cmd(ctx, node);
    else if (node->type == RL_FUNDEC)
        return rl_exec_fundec(ctx, node);

    assert(0);
}
