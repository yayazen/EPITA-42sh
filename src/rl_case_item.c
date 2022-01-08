#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_case_item(struct rl_state *s)
{
    struct rl_exectree *node;
    struct rl_exectree *child;

    /* ['('] WORD */
    if (rl_accept(s, T_LPAR) == true)
    {
        if (rl_expect(s, T_WORD) <= 0)
            return -s->err;
    }
    /* WORD */
    else if (rl_accept(s, T_WORD) <= 0)
        return -s->err;

    assert(node = rl_exectree_new(RL_CASE_ITEM));
    assert(child = rl_exectree_new(RL_WORD));

    node->child = child;
    child->attr.word = strdup(vec_cstring(&s->word));

    /* ('|' WORD)* */
    while (rl_accept(s, T_PIPE) == true)
    {
        if (rl_expect(s, T_WORD) <= 0)
        {
            rl_exectree_free(node);
            s->node = NULL;
            return -s->err;
        }

        struct rl_exectree *next = rl_exectree_new(RL_WORD);
        assert(next != NULL);
        next->attr.word = strdup(vec_cstring(&s->word));

        child->sibling = next;
        child = child->sibling;
    }

    /* ')' */
    if (rl_expect(s, T_RPAR) <= 0)
    {
        rl_exectree_free(node);
        s->node = NULL;
        return -s->err;
    }

    /* ('\n')* */
    while (rl_accept(s, T_LF) == true)
        ;

    /* [compound_list] */
    int compound_list = rl_compound_list(s);
    if (compound_list < 0)
    {
        rl_exectree_free(node);
        s->node = NULL;
        return -s->err;
    }
    else if (compound_list == true)
        child->sibling = s->node;

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}