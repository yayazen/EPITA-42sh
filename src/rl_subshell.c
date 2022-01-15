#include <assert.h>

#include "ctx.h"
#include "rule.h"
#include "symtab.h"
#include "token.h"

int rl_subshell(struct rl_state *s)
{
    /* '(' */
    if (rl_accept(s, T_LPAR) != true)
        return -s->err;

    s->flag |= PARSER_LINE_START | LEX_CMDSTART;
    /* compound_list ')' */
    if (rl_compound_list(s) <= 0 || rl_expect(s, T_RPAR) <= 0)
    {
        rl_exectree_free(s->node);
        s->node = NULL;
        return -s->err;
    }
    s->flag &= ~PARSER_LINE_START;

    // Encapsulate the command
    struct rl_exectree *child = rl_exectree_new(RL_SUBSHELL);
    child->child = s->node;
    s->node = child;

    return true;
}

int rl_exec_subshell(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node->type == RL_SUBSHELL);

    struct ctx child_ctx = *ctx;
    child_ctx.alloc_list = NULL;
    child_ctx.loop_jump = NULL;
    child_ctx.st = symtab_clone(ctx->st);

    jmp_buf exit_buff;
    volatile int jmpval;
    jmpval = setjmp(exit_buff);
    if (jmpval != 0)
        ;
    else
    {
        child_ctx.exit_jump = &exit_buff;

        rl_exec_compound_list(&child_ctx, node->child);
    }

    symtab_free(child_ctx.st);
    return *ctx->exit_status;
}