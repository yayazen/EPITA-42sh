#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_case(struct rl_state *s)
{
    struct rl_exectree *node;

    /* Case WORD */
    if (rl_accept(s, T_CASE) <= 0 || rl_expect(s, T_WORD) <= 0)
        return -s->err;

    assert(node = rl_exectree_new(RL_CASE));
    node->attr.word = strdup(vec_cstring(&s->word));

    s->flag |= LEX_CMDSTART;

    /* ('\n')* */
    while (rl_accept(s, T_LF) == true)
        ;

    /* 'in' */
    if (rl_expect(s, T_IN) <= 0)
    {
        rl_exectree_free(node);
        return -s->err;
    }

    s->flag |= LEX_CMDSTART;

    /* ('\n')* */
    while (rl_accept(s, T_LF) == true)
        ;

    /* [case_clause] */
    s->flag |= PARSER_LINE_START;
    int case_clause = rl_case_clause(s);
    if (case_clause < 0)
    {
        rl_exectree_free(node);
        return -s->err;
    }
    node->child = case_clause == false ? NULL : s->node;

    if (rl_expect(s, T_ESAC) <= 0)
    {
        s->node = node;
        return -s->err;
    }

    s->node = node;

    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_case(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node && ctx && node->type == RL_CASE);

    if (node->child)
        rl_exec_case_clause(ctx, node->child, node->attr.word);

    return 0;
}