#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_case_clause(struct rl_state *s)
{
    struct rl_exectree *node;

    /* case_item */
    if (rl_case_item(s) <= 0)
        return -s->err;

    assert(node = rl_exectree_new(RL_CASE_CLAUSE));
    node->child = s->node;

    struct rl_exectree *child = node->child;

    s->flag |= LEX_CMDSTART;

    /* [;;] */
    while (rl_accept(s, T_DSEMICOL) == true)
    {
        /* ('\n')* */
        while (rl_accept(s, T_LF) == true)
            ;

        /* case_item */
        if (rl_case_item(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;

        s->flag |= LEX_CMDSTART;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_case_clause(const struct ctx *ctx, struct rl_exectree *node,
                        const char *word)
{
    assert(ctx && node && node->type == RL_CASE_CLAUSE && word);
    struct rl_exectree *child = node->child;

    // Process each case item
    while (child)
    {
        assert(child->type == RL_CASE_ITEM);

        // Check if a word in the item is matched
        struct rl_exectree *item_search = child->child;
        int found = false;
        while (item_search && item_search->type == RL_WORD)
        {
            found = found || strcmp(item_search->attr.word, word) == 0
                || strcmp(item_search->attr.word, "*") == 0;

            item_search = item_search->sibling;
        }

        if (found && item_search && item_search->type == RL_COMPOUND_LIST)
            rl_exec_compound_list(ctx, item_search);

        if (found)
            break;

        child = child->sibling;
    }

    return 0;
}