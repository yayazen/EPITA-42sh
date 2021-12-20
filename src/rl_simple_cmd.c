#define _DEFAULT_SOURCE
#include <stdlib.h>
#undef _DEFAULT_SOURCE

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
    while (arg != NULL)
    {
        if (arg->type == RL_ASSIGN_WORD)
        {
            char *str = strdup(arg->attr.word);
            char *eq = strchr(str, '=');
            *eq = '\0';
            symtab_add(symtab, str, KV_WORD, symexp_word(symtab, eq + 1));

            free(str);
        }
        arg = arg->sibling;
    }
}

static void __apply_env(struct symtab *st, struct rl_exectree *arg)
{
    assert(st);

    /* First, remove any inherited environment variable */
    clearenv();

    /* Second, apply all exported variables */
    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        while (kv)
        {
            if (kv->type == KV_WORD && kv->value.word.exported)
                setenv(kv->key, kv->value.word.word, 1);
            kv = kv->next;
        }
    }

    /* Lastly, apply all symbols specified in command */
    while (arg != NULL)
    {
        if (arg->type == RL_ASSIGN_WORD)
        {
            char *str = strdup(arg->attr.word);
            char *eq = strchr(str, '=');
            *eq = '\0';
            char *val = symexp_word(symtab, eq + 1);

            setenv(str, val, 1);

            free(str);
            free(val);
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
            node->attr.cmd.status = blt(argv, symtab);
        }
        else
        {
            node->attr.cmd.pid = fork();
            if (node->attr.cmd.pid == 0)
            {
                __apply_env(symtab, node->child);

                execvp(argv[0], argv);
                fprintf(stderr, PACKAGE ": %s: command not found...\n",
                        argv[0]);
                exit(127);
            }
        }
    }

    else
    {
        __add_symbol(node->child);
    }

    for (int i = 0; i < 3; i++)
    {
        if (__redirect(savedfd[i], i, true) != 0)
            return EXECUTION_ERROR;
    }

    return (node->attr.cmd.pid == -1 && !blt) ? EXECUTION_ERROR : NO_ERROR;
}
