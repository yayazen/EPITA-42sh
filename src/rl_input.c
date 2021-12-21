#include <assert.h>
#include <utils/error.h>

#include "rule.h"
#include "token.h"

int rl_input(struct rl_state *s)
{
    if (rl_accept(s, T_LF) == true || rl_accept(s, T_EOF) == true)
        return true;

    if (s->err != NO_ERROR || rl_list(s) <= 0)
        return -s->err;

    if (rl_accept(s, T_EOF) == true || rl_accept(s, T_LF) == true)
        return true;

    return (s->err != NO_ERROR) ? -s->err : false;
}

int rl_exec_input(struct rl_exectree *node, const struct ctx *ctx)
{
    return (node) ? rl_exec_list(node, ctx) : 0;
}
