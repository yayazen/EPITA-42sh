#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <utils/error.h>

#include "constants.h"
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
            return -s->err;
        s->flag &= ~PARSER_LINE_START;
    }
    /* '(' compound_list ')' */
    else if (rl_accept(s, T_LPAR) == true)
    {
        s->flag |= PARSER_LINE_START;
        if (rl_compound_list(s) <= 0 || rl_expect(s, T_RPAR) <= 0)
            return -s->err;
        s->flag &= ~PARSER_LINE_START;
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

    else
        return false;

    if (!(node = rl_exectree_new(RL_SHELL_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = s->node;
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

int rl_exec_shell_cmd(struct rl_exectree *node, const struct ctx *ctx)
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

    int type = node->child->type;
    if (type == RL_COMPOUND_LIST)
        node->attr.cmd.status = rl_exec_compound_list(node->child, ctx);
    else if (type == RL_IF)
        node->attr.cmd.status = rl_exec_if_clause(node->child, ctx);
    else if (type == RL_WHILE)
        node->attr.cmd.status = rl_exec_while(node->child, ctx);
    else if (type == RL_UNTIL)
        node->attr.cmd.status = rl_exec_until(node->child, ctx);

    for (int i = 0; i < 3; i++)
    {
        if (__redirect(savedfd[i], i, true) != 0)
            return EXECUTION_ERROR;
    }

    return NO_ERROR;
}
