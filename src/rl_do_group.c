#include "rule.h"
#include "token.h"

int rl_do_group(struct rl_state *s)
{
    if (rl_expect(s, T_DO, RL_NORULE) <= 0 || rl_compound_list(s) <= 0
        || rl_expect(s, T_DONE, RL_NORULE) <= 0)
        return -s->err;

    return true;
}

int rl_exec_do_group(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_COMPOUND_LIST);
    return rl_exec_compound_list(ast);
}