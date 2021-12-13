#include <assert.h>

#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    return rl_shell_cmd(s) || rl_simple_cmd(s);
}

int rl_exec_cmd(struct rl_ast *ast)
{
    assert(ast);

    switch (ast->type)
    {
    case RL_SIMPLE_CMD:
        return rl_exec_simple_cmd(ast);
    case RL_SHELL_CMD:
        return rl_exec_shell_cmd(ast);
    default:
        assert(0);
    }
}
