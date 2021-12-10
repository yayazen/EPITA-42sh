#include "rule.h"

int rl_cmd(struct rl_state *s)
{
    return rl_simple_cmd(s);
}

int rl_exec_cmd(struct rl_ast *ast)
{
    return rl_exec_simple_cmd(ast);
}
