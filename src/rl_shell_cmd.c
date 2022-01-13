#include <assert.h>
#include <fcntl.h>
#include <setjmp.h>
#include <unistd.h>
#include <utils/error.h>

#include "constants.h"
#include "ctx.h"
#include "rule.h"
#include "token.h"

int rl_shell_cmd(struct rl_state *s)
{
    struct rl_exectree *node;

    /* '{' compound_list '}' */
    if (rl_accept(s, T_LBRACE) == true)
    {
        s->flag |= PARSER_LINE_START;
        if (rl_compound_list(s) <= 0 || rl_expect(s, T_RBRACE) <= 0)
        {
            rl_exectree_free(s->node);
            s->node = NULL;
            return -s->err;
        }
        s->flag &= ~PARSER_LINE_START;
    }
    /* '(' compound_list ')' */
    else if (rl_accept(s, T_LPAR) == true)
    {
        s->flag |= PARSER_LINE_START;
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
    }

    /* rule_if */
    else if (rl_if_clause(s) == true)
        ;

    /* rule_while */
    else if (rl_while(s) == true)
        ;

    /* rule_until */
    else if (rl_until(s) == true)
        ;

    /* rule_for */
    else if (rl_for(s) == true)
        ;

    /* rule_case */
    else if (rl_case(s) == true)
        ;

    else
        return false;

    if (!(node = rl_exectree_new(RL_SHELL_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = s->node;

    /* (redirection)* */
    struct rl_exectree *child = node->child;
    while (rl_redirection(s) == true)
    {
        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return true;
}

static inline int __redirect(int oldfd, int newfd, int closefd)
{
    if (oldfd == newfd)
        return 0;
    if (dup2(oldfd, newfd) == -1)
        return -1;
    if (closefd && close(oldfd) == -1)
        return -1;
    return 0;
}

static int __exec_subshell(const struct ctx *ctx, struct rl_exectree *node)
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

int rl_exec_shell_cmd(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_SHELL_CMD);

    int savedfd[3];
    for (int i = 0; i < 3; i++)
    {
        if ((savedfd[i] = dup(i)) == -1)
            return EXECUTION_ERROR;
        fcntl(savedfd[i], F_SETFD, FD_CLOEXEC);
        if (__redirect(node->attr.cmd.fd[i], i, i == 0) != 0)
            return EXECUTION_ERROR;
    }

    struct rl_exectree *redirs_node = node->child->sibling;
    while (redirs_node)
    {
        assert(rl_exec_redirection(redirs_node) == NO_ERROR);
        redirs_node = redirs_node->sibling;
    }

    int type = node->child->type;
    if (type == RL_COMPOUND_LIST)
        node->attr.cmd.status = rl_exec_compound_list(ctx, node->child);
    else if (type == RL_IF)
        node->attr.cmd.status = rl_exec_if_clause(ctx, node->child);
    else if (type == RL_WHILE)
        node->attr.cmd.status = rl_exec_while(ctx, node->child);
    else if (type == RL_FOR)
        node->attr.cmd.status = rl_exec_for(ctx, node->child);
    else if (type == RL_UNTIL)
        node->attr.cmd.status = rl_exec_until(ctx, node->child);
    else if (type == RL_CASE)
        node->attr.cmd.status = rl_exec_case(ctx, node->child);
    else if (type == RL_SUBSHELL)
        node->attr.cmd.status = __exec_subshell(ctx, node->child);

    for (int i = 0; i < 3; i++)
    {
        if (__redirect(savedfd[i], i, true) != 0)
            return EXECUTION_ERROR;
    }

    return NO_ERROR;
}
