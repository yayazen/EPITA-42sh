#include <utils/vec.h>

#include "rule.h"
#include "token.h"

int rl_prefix(struct rl_state *s)
{
    struct rl_exectree *node;

    if (rl_redirection(s) == true)
        return true;

    if (rl_accept(s, T_ASSIGN_WORD) <= 0)
        return -s->err;

    node = rl_exectree_new(RL_ASSIGN_WORD);
    if (!node || !(node->attr.word = strdup(vec_cstring(&s->word))))
        return -(s->err = PARSER_ERROR);
    s->node = node;
    return 1;
}
