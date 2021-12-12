#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    return rl_simple_cmd(s) == true ? true : rl_shell_command(s);
}

int rl_exec_cmd(struct rl_ast *ast)
{
    if (ast->type == RL_SIMPLE_CMD)
        return rl_exec_simple_cmd(ast);
    else
        return rl_exec_shell_command(ast);
}
