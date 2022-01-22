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
    int res;
    while ((res = rl_prefix(s)) == true)
    {
        *childptr = s->node;
        childptr = &(*childptr)->sibling;
    }

    if (res < 0)
    {
        rl_exectree_free(node);
        return -s->err;
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

static inline void __push_args(const struct ctx *ctx, struct rl_exectree *arg,
                               struct list *l)
{
    for (; arg != NULL; arg = arg->sibling)
    {
        if (arg->type == RL_WORD)
        {
            symexp_word(ctx, arg->attr.word, l);
        }
        else if (arg->type == RL_REDIRECTION)
            assert(rl_exec_redirection(ctx, arg) == NO_ERROR);
        else if (arg->type == RL_ASSIGN_WORD)
        {
            ; /* used later */
        }
    }
}

static inline void __add_symbol(const struct ctx *ctx, struct rl_exectree *arg)
{
    while (arg != NULL)
    {
        if (arg->type == RL_ASSIGN_WORD)
        {
            char *str = strdup(arg->attr.word);
            char *eq = strchr(str, '=');
            *eq = '\0';
            symtab_add(ctx->st, str, KV_WORD,
                       symexp_word_single_result(ctx, eq + 1));

            free(str);
        }
        arg = arg->sibling;
    }
}

static void __apply_env(const struct ctx *ctx, struct rl_exectree *arg)
{
    assert(ctx);

    /* First, remove any inherited environment variable */
    clearenv();

    /* Second, apply all exported variables */
    for (size_t i = 0; i < ctx->st->capacity; i++)
    {
        struct kvpair *kv = ctx->st->data[i];
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
            char *val = symexp_word_single_result(ctx, eq + 1);

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
    __push_args(ctx, node->child, args);
    list_push(args, NULL);

    struct kvpair *kv;

    // Run command (if any)
    if (args->size > 1)
    {
        // Create a child context to store arguments list
        // This is needed to execute builtins or functions
        CTX_CHILD_FOR_LIST(ctx, child_ctx, args)

        // Builtin
        if ((blt = builtin_find(args->data[0])))
        {
            node->attr.cmd.status = blt(&child_ctx, args->data);
        }

        // Function
        else if ((kv = symtab_lookup(ctx->st, args->data[0], KV_FUNC)) != NULL)
        {
            child_ctx.loop_jump = NULL;
            child_ctx.program_args = args->data;
            child_ctx.program_args_count = args->size - 1;

            // We need to clone the function because the function entry in the
            // symtable could change while executing the function (be changed or
            // deleted)
            struct rl_exectree *func = rl_exectree_clone(kv->value.func);

            CTX_CHILD_FOR_EXECTREE(&child_ctx, child2_ctx, func);
            child2_ctx.flags |= IN_FUNCTION;
            node->attr.cmd.status = rl_exec_shell_cmd(&child2_ctx, func);
            rl_exectree_free(func);
        }

        // Standard command
        else
        {
            node->attr.cmd.pid = fork();
            if (node->attr.cmd.pid == 0)
            {
                __apply_env(ctx, node->child);

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
        __add_symbol(ctx, node->child);
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
