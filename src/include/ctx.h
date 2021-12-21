#pragma once

#include <setjmp.h>

struct symtab;

/** \brief execution context for the tree */
struct ctx
{
    /* symbols table */
    struct symtab *st;

    /* last exit status */
    int *exit_status;

    /* exit jump */
    jmp_buf *exit_jump;

    /* other fields for continue and break builtins */
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
    };

    return st;
}
