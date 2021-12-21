#pragma once

#include <setjmp.h>
#include <stddef.h>

struct symtab;

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

    jmp_buf *jump;
};

/** \brief execution context for the tree */
struct ctx
{
    /* symbols table */
    struct symtab *st;

    /* last exit status */
    int *exit_status;

    /* exit jump */
    jmp_buf *exit_jump;

    /* break & continue on loops */
    struct ctx_jmp *loop_jump;
};

/** \brief create a new execution context.
 * This function should be called only from parser */
static inline struct ctx ctx_new(struct symtab *table, int *exit_status,
                                 jmp_buf *exit_jump)
{
    struct ctx st = {
        .st = table,
        .exit_status = exit_status,
        .exit_jump = exit_jump,
        .loop_jump = NULL,
    };

    return st;
}

/** \brief create a child context for a loop execution */
static inline struct ctx ctx_add_jump(const struct ctx *parent,
                                      struct ctx_jmp *node, jmp_buf *jmp)
{
    struct ctx child = *parent;

    node->jump = jmp;
    node->next = parent->loop_jump;
    child.loop_jump = node;

    return child;
}