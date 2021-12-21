#pragma once

struct symtab;

struct ctx_word
{
    /* key */
    char *key;

    /* associated value */
    char *value;

    /* next token */
    struct ctx_word *next;
};

/** \brief execution context for the tree */
struct ctx
{
    /* symbols table */
    struct symtab *st;

    /* overwritten words for tree (eg. in for loops)
        This list must be "applied" in reverse order (from
        the end to the beggining, because of variables priority)
        This can be done easily using a recursive function */
    struct ctx_word *word;

    /* over fields for continue, break and exit builtins */
};

/** \brief create a new execution context.
 * This function should be called only from parser */
static inline struct ctx ctx_new(struct symtab *table)
{
    struct ctx st = { .st = table, .word = NULL };

    return st;
}

/** \brief create a child context with an additional word */
static inline struct ctx ctx_add_word(const struct ctx *parent,
                                      struct ctx_word *word)
{
    struct ctx child_ctx = *parent;

    word->next = child_ctx.word;
    child_ctx.word = word;

    return child_ctx;
}