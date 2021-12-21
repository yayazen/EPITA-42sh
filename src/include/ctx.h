#pragma once

struct symtab;

/** \brief execution context for the tree */
struct ctx
{
    /* symbols table */
    struct symtab *st;

    /* other fields for continue, break and exit builtins */
};

/** \brief create a new execution context.
 * This function should be called only from parser */
static inline struct ctx ctx_new(struct symtab *table)
{
    struct ctx st = { .st = table };

    return st;
}
