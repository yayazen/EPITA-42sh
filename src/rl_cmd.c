#include <assert.h>

#include "constants.h"
#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    s->flag |= LEX_CMDSTART;
    return rl_shell_cmd(s) || rl_fundec(s) || rl_simple_cmd(s);
}

int rl_exec_cmd(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node);

    if (node->type == RL_SIMPLE_CMD)
        return rl_exec_simple_cmd(ctx, node);
    else if (node->type == RL_SHELL_CMD)
        return rl_exec_shell_cmd(ctx, node);

    assert(0);
}
