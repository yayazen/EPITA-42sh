#include <assert.h>
#include <utils/error.h>

#include "rule.h"
#include "token.h"

int rl_input(struct rl_state *s)
{
    if (rl_accept(s, T_LF, RL_NORULE) == true
        || rl_accept(s, T_EOF, RL_NORULE) == true)
        return true;

    if (s->err != NO_ERROR || rl_list(s) <= 0)
        return -s->err;

    if (rl_accept(s, T_EOF, RL_NORULE) == true
        || rl_accept(s, T_LF, RL_NORULE) == true)
        return true;

    return (s->err != NO_ERROR) ? -s->err : false;
}

int rl_exec_input(struct rl_exectree *node)
{
    return (node) ? rl_exec_list(node) : 0;
}
