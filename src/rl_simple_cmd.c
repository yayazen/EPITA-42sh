#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "builtins.h"
#include "constants.h"
#include "rule.h"
#include "token.h"

#define ARG_MAX 256

int rl_simple_cmd(struct rl_state *s)
{
    struct rl_ast *node;

    /* WORD */
    s->flag |= LEX_CMDSTART;
    if (rl_accept(s, T_WORD, RL_WORD) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_SIMPLE_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* WORD* */
    s->flag &= ~LEX_CMDSTART;
    while (rl_accept(s, T_WORD, RL_WORD) == true)
    {
        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : 1;
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

int rl_exec_simple_cmd(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_SIMPLE_CMD);

    char *argv[ARG_MAX] = { 0 };
    struct rl_ast *arg = ast->child;
    for (int i = 0; arg != NULL; i++, arg = arg->sibling)
        argv[i] = arg->word;

    ast->pid = fork();
    assert(ast->pid != -1);
    if (ast->pid == 0)
    {
        assert(__redirect(ast->fd[0], STDIN_FILENO, true) == 0);
        assert(__redirect(ast->fd[1], STDOUT_FILENO, false) == 0);
        execvp(argv[0], argv);
        fprintf(stderr, PACKAGE ": %s: command not found...\n", argv[0]);
        exit(127);
    }
    return 0;
}
