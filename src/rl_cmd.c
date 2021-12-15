#include <assert.h>

#include "constants.h"
#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    s->flag |= LEX_CMDSTART;
    return rl_shell_cmd(s) || rl_simple_cmd(s);
}

int rl_exec_cmd(struct rl_ast *ast)
{
    assert(ast);

    if (ast->type == RL_SIMPLE_CMD)
        return rl_exec_simple_cmd(ast);
    else if (ast->type == RL_SHELL_CMD)
        return rl_exec_shell_cmd(ast);

    assert(0);
}
