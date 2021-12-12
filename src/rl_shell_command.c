#include <rule.h>

int rl_shell_command(struct rl_state *s)
{
    return rl_rule_if(s);
}

int rl_exec_shell_command(struct rl_ast *s)
{
    return rl_exec_rule_if(s);
}