#include "rule.h"
#include "token.h"

int rl_and_or(struct rl_state *s)
{
    return rl_pipeline(s);
}

int rl_exec_and_or(struct rl_ast *ast)
{
    return rl_exec_pipeline(ast);
}