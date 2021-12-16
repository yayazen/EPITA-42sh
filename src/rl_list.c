#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_list(struct rl_state *s)
{
    struct rl_exectree *node;

    /* and_or */
    if (rl_and_or(s) <= 0)
        return -s->err;
    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_LIST)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* ((';'|'&') and_or)* [';'|'&'] */
    while (rl_accept(s, T_SEMICOL) == true || rl_accept(s, T_AND) == true)
    {
        if (rl_and_or(s) <= 0)
            break;
        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_list(struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_LIST);

    int status;
    node = node->child;
    do
    {
        status = rl_exec_and_or(node);
    } while ((node = node->sibling));

    return status;
}
