#pragma once

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct symtab;

#include "list.h"

enum
{
    JMP_NOOP,
    JMP_CONTINUE,
    JMP_BREAK
};

/** \brief loop jump */
struct ctx_jmp
{
    /* next jump */
    struct ctx_jmp *next;

    /* context level the jump leads to */
    int level;

    /* jump buffer */
    jmp_buf *jump;
};

/** \brief holds a pointer to a list to free in case of jump */
struct ctx_str_list
{
    /* next allocated memory element */
    struct ctx_str_list *next;

    /* context level this list is in */
    int level;

    /* the allocated list */
    struct list *list;
};

/** \brief execution context for the tree */
struct ctx
{
    /* recursion level of context */
    int level;

    /* symbols table */
    struct symtab *st;

    /* last exit status */
    int *exit_status;

    /* specify whether we are in an interactive shell or not */
    int is_interactive;

    /* exit jump */
    jmp_buf *exit_jump;

    /* break & continue on loops */
    struct ctx_jmp *loop_jump;

    /* list to free in case of jump */
    struct ctx_str_list *str_list;
};

/** \brief create a new execution context.
 * This function should be called only from parser */
static inline struct ctx ctx_new(struct symtab *table, int *exit_status,
                                 jmp_buf *exit_jump)
{
    struct ctx st = {
        .level = 1,
        .st = table,
        .exit_status = exit_status,
        .is_interactive = false,
        .exit_jump = exit_jump,
        .loop_jump = NULL,
        .str_list = NULL,
    };

    return st;
}

/** \brief create a child context for a loop execution */
static inline struct ctx ctx_add_jump(const struct ctx *parent,
                                      struct ctx_jmp *node, jmp_buf *jmp)
{
    struct ctx child = *parent;
    child.level++;

    node->level = child.level;
    node->jump = jmp;
    node->next = parent->loop_jump;

    child.loop_jump = node;

    return child;
}

/** \brief create a child context to register a list to free */
static inline struct ctx ctx_add_list(const struct ctx *parent,
                                      struct ctx_str_list *node,
                                      struct list *list)
{
    struct ctx child = *parent;
    child.level++;

    node->level = child.level;
    node->list = list;
    node->next = parent->str_list;

    child.str_list = node;

    return child;
}

/** \brief free allocated memory below a certain given context */
static inline void ctx_free_allocated_memory(const struct ctx *ctx, int level)
{
    struct ctx_str_list *l = ctx->str_list;
    while (l && l->level > level)
    {
        list_free(l->list);
        l = l->next;
    }
}