#include <assert.h>

#include "constants.h"
#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    s->flag |= LEX_CMDSTART;
    return rl_shell_cmd(s) || rl_simple_cmd(s);
}

int rl_exec_cmd(struct rl_exectree *node, const struct ctx *ctx)
{
    assert(node);

    if (node->type == RL_SIMPLE_CMD)
        return rl_exec_simple_cmd(node, ctx);
    else if (node->type == RL_SHELL_CMD)
        return rl_exec_shell_cmd(node, ctx);

    assert(0);
}
