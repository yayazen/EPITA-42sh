#include "rule.h"

int rl_compound_list(struct rl_state *s)
{
    return rl_list(s);
}

int rl_exec_compound_list(struct rl_ast *s)
{
    return rl_exec_list(s);
}