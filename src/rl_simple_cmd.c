#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "builtins.h"
#include "constants.h"
#include "rule.h"
#include "symexp.h"
#include "symtab.h"
#include "token.h"

#define ARG_MAX 256

struct symtab *symtab = NULL;

int rl_simple_cmd(struct rl_state *s)
{
    struct rl_exectree *node;
    if (!(node = rl_exectree_new(RL_SIMPLE_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    struct rl_exectree **childptr = &node->child;

    s->flag |= LEX_CMDSTART;

    /* prefix* */
    while (rl_prefix(s) == true)
    {
        *childptr = s->node;
        childptr = &(*childptr)->sibling;
    }

    /* element */
    if (rl_element(s) <= 0)
    {
        if (!node->child)
        {
            rl_exectree_free(node);
            return false;
        }
        s->node = node;
        return (s->err != NO_ERROR) ? -s->err : true;
    }

    *childptr = s->node;
    childptr = &(*childptr)->sibling;
    /* element* */
    s->flag &= ~LEX_CMDSTART;
    while (rl_element(s) == true)
    {
        *childptr = s->node;
        childptr = &(*childptr)->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
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

static inline int __push_args(struct rl_exectree *arg, char **argv)
{
    int i;
    for (i = 0; arg != NULL; arg = arg->sibling)
    {
        if (arg->type == RL_WORD)
        {
            char *expword = symexp_word(symtab, arg->attr.word);
            assert(expword != NULL);
            free(arg->attr.word);
            arg->attr.word = expword;
            argv[i++] = arg->attr.word;
        }
        else if (arg->type == RL_REDIRECTION)
            assert(rl_exec_redirection(arg) == NO_ERROR);
        else if (arg->type == RL_ASSIGN_WORD)
        {
            ;
        }
    }

    return i > 0;
}

static inline void __add_symbol(struct rl_exectree *arg)
{
    if (!symtab)
        symtab = symtab_new();

    while (arg != NULL)
    {
        if (arg->type == RL_ASSIGN_WORD)
        {
            char *str = strdup(arg->attr.word);
            char *eq = strchr(str, '=');
            *eq = '\0';
            symtab_add(symtab, str, KV_WORD, eq + 1);

            free(str);
        }
        arg = arg->sibling;
    }
}

int rl_exec_simple_cmd(struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_SIMPLE_CMD);

    int savedfd[3];
    for (int i = 0; i < 3; i++)
    {
        if ((savedfd[i] = dup(i)) == -1)
            return EXECUTION_ERROR;
        fcntl(savedfd[i], F_SETFD, FD_CLOEXEC);
        if (__redirect(node->attr.cmd.fd[i], i, i == 0) != 0)
            return EXECUTION_ERROR;
    }

    builtin_def blt;
    char *argv[ARG_MAX] = { 0 };
    if (__push_args(node->child, argv))
    {
        if ((blt = builtin_find(argv[0])))
        {
            node->attr.cmd.status = blt(argv);
        }
        else
        {
            node->attr.cmd.pid = fork();
            if (node->attr.cmd.pid == 0)
            {
                execvp(argv[0], argv);
                fprintf(stderr, PACKAGE ": %s: command not found...\n",
                        argv[0]);
                exit(127);
            }
        }
    }

    for (int i = 0; i < 3; i++)
    {
        if (__redirect(savedfd[i], i, true) != 0)
            return EXECUTION_ERROR;
    }

    __add_symbol(node->child);

    return (node->attr.cmd.pid == -1 && !blt) ? EXECUTION_ERROR : NO_ERROR;
}
