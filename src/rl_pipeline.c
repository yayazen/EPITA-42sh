#include "rule.h"
#include "token.h"

int rl_pipeline(struct rl_state *s)
{
    return rl_cmd(s);
}

int rl_exec_pipeline(struct rl_ast *ast)
{
    return rl_exec_cmd(ast);
}