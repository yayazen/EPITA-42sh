#include <assert.h>
#include <setjmp.h>

#include "constants.h"
#include "ctx.h"
#include "rule.h"
#include "token.h"

int rl_while(struct rl_state *s)
{
    struct rl_exectree *node;

    /* While compound_list */
    if (rl_accept(s, T_WHILE) <= 0)
        return -s->err;
    s->flag |= PARSER_LINE_START;
    if (rl_compound_list(s) <= 0)
    {
        s->flag &= ~PARSER_LINE_START;
        return -s->err;
    }

    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_WHILE)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* do_group */
    s->node = NULL;
    if (rl_do_group(s) <= 0)
    {
        s->flag &= ~PARSER_LINE_START;
        rl_exectree_free(node);
        return -s->err;
    }

    s->flag &= ~PARSER_LINE_START;
    child->sibling = s->node;
    s->node = node;

    return true;
}

int rl_exec_while(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node && node->type == RL_WHILE);

    while (rl_exec_compound_list(ctx, node->child) == 0)
    {
        jmp_buf jump_buffer;
        struct ctx_jmp jmp_node;
        volatile int val;

        val = setjmp(jump_buffer);

        if (val == JMP_CONTINUE)
        {
            continue;
        }
        else if (val == JMP_BREAK)
        {
            break;
        }
        else if (val == JMP_NOOP)
        {
            struct ctx child_ctx = ctx_add_jump(ctx, &jmp_node, &jump_buffer);

            rl_exec_compound_list(&child_ctx, node->child->sibling);
        }
        else
            assert(0);
    }

    return 0;
}
