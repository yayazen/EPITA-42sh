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
#include "ctx.h"
#include "list.h"
#include "rule.h"
#include "symexp.h"
#include "symtab.h"
#include "token.h"

#define ARG_MAX 256

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

static inline void __push_args(struct rl_exectree *arg, struct list *l,
                               struct symtab *symtab)
{
    for (; arg != NULL; arg = arg->sibling)
    {
        if (arg->type == RL_WORD)
        {
            char *expword = symexp_word(symtab, arg->attr.word);
            assert(expword != NULL);
            list_push(l, expword);
        }
        else if (arg->type == RL_REDIRECTION)
            assert(rl_exec_redirection(arg) == NO_ERROR);
        else if (arg->type == RL_ASSIGN_WORD)
        {
            ; /* used later */
        }
    }
}

static inline void __add_symbol(struct rl_exectree *arg, struct symtab *symtab)
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
            char *val = symexp_word(st, eq + 1);

            setenv(str, val, 1);

            free(str);
            free(val);
        }
        arg = arg->sibling;
    }
}

int rl_exec_simple_cmd(const struct ctx *ctx, struct rl_exectree *node)
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

    builtin_def blt = NULL;

    // Parse command arguments
    struct list *args = list_new(ARG_MAX);
    __push_args(node->child, args, ctx->st);
    list_push(args, NULL);

    // Run command (if any)
    if (args->size > 1)
    {
        if ((blt = builtin_find(args->data[0])))
        {
            struct ctx_str_list list_el;
            struct ctx child_ctx = ctx_add_list(ctx, &list_el, args);

            node->attr.cmd.status = blt(&child_ctx, args->data);
        }
        else
        {
            node->attr.cmd.pid = fork();
            if (node->attr.cmd.pid == 0)
            {
                __apply_env(ctx->st, node->child);

                execvp(args->data[0], args->data);
                fprintf(stderr, PACKAGE ": %s: command not found...\n",
                        args->data[0]);
                exit(127);
            }
        }
    }

    // Or save assigned variables
    else
    {
        __add_symbol(node->child, ctx->st);
    }

    // Free memory
    list_free(args);

    for (int i = 0; i < 3; i++)
    {
        if (__redirect(savedfd[i], i, true) != 0)
            return EXECUTION_ERROR;
    }

    return (node->attr.cmd.pid == -1 && !blt) ? EXECUTION_ERROR : NO_ERROR;
}
