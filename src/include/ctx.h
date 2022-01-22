/**
 * \file ctx.h
 * \brief AST Execution context.
 *
 * The context managed in this file is used to:
 *
 * * Store a reference to the symbols table
 * * Store some flags (about execution mode / interactivness)
 * * Give access to program / Function arguments
 * * Store `exit`, `continue` and `break` jump destinations
 * * Give reference to memory to free in case of jump.
 *
 * Some functions / macros are provided to create child context based on parent
 * context. The parent and child context MUST be stored on the stack and **NOT**
 * on the heap.
 *
 * Context are specified as `const` program arguments thus, a new child context
 * must be created each time an execution node needs to modify the context for
 * its children.
 */

#pragma once

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct symtab;

#include "constants.h"
#include "list.h"
#include "rule_exec.h"

enum
{
    JMP_NOOP,
    JMP_CONTINUE,
    JMP_BREAK
};

/** \brief context loop jump */
struct ctx_jmp
{
    /** \brief next jump */
    struct ctx_jmp *next;

    /** \brief context level the jump leads to */
    int level;

    /** \brief jump buffer */
    jmp_buf *jump;
};

enum ctx_alloc_type
{
    /** \brief the allocation referenced by this node is a list of strings */
    ALLOC_LIST,

    /** \brief the allocation referenced by this node is an execution tree (a
       function for example) */
    ALLOC_EXECTREE,
};

/** \brief holds a pointer to a list to free in case of jump */
struct ctx_allocation_list
{
    /** \brief next allocated memory element */
    struct ctx_allocation_list *next;

    /** \brief context level this list is in */
    int level;

    /** \brief the type of the element to free */
    enum ctx_alloc_type type;

    /** \brief a pointer on the begining of allocated data */
    void *data;
};

/** \brief execution context for the tree */
struct ctx
{
    /** \brief recursion level of context */
    int level;

    /** \brief symbols table */
    struct symtab *st;

    /** \brief last exit status */
    int *exit_status;

    /** \brief context flags */
    int flags;

    /** \brief Number of program arguments */
    int program_args_count;

    /** \brief Pointer on the first program argument */
    char **program_args;

    /** \brief exit jump */
    jmp_buf *exit_jump;

    /** \brief break & continue on loops */
    struct ctx_jmp *loop_jump;

    /** \brief list of allocations to free in case of jump */
    struct ctx_allocation_list *alloc_list;
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
        .flags = 0,
        .program_args_count = 0,
        .program_args = NULL,
        .exit_jump = exit_jump,
        .loop_jump = NULL,
        .alloc_list = NULL,
    };

    return st;
}

/** \brief Filter accepted flags in contex */
static inline int ctx_filter_flags(int flags)
{
    return flags & (MODE_INPUT | MODE_COMMAND | MODE_SCRIPT | IS_INTERACTIVE);
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
static inline struct ctx ctx_add_alloc(const struct ctx *parent,
                                       struct ctx_allocation_list *node,
                                       int type, void *data)
{
    struct ctx child = *parent;
    child.level++;

    node->level = child.level;
    node->data = data;
    node->type = type;
    node->next = parent->alloc_list;

    child.alloc_list = node;

    return child;
}

/** \brief free allocated memory below a certain given context */
static inline void ctx_free_allocated_memory(const struct ctx *ctx, int level)
{
    struct ctx_allocation_list *l = ctx->alloc_list;
    while (l && l->level > level)
    {
        if (l->type == ALLOC_LIST)
            list_free(l->data);
        else if (l->type == ALLOC_EXECTREE)
            rl_exectree_free(l->data);

        l = l->next;
    }
}

#define CTX_CHILD_FOR_LIST(parent_ctx, child_ctx, list)                        \
    struct ctx_allocation_list list_ctx_node;                                  \
    struct ctx child_ctx =                                                     \
        ctx_add_alloc(parent_ctx, &list_ctx_node, ALLOC_LIST, list);

#define CTX_CHILD_FOR_EXECTREE(parent_ctx, child_ctx, tree)                    \
    struct ctx_allocation_list list_ctx_node;                                  \
    struct ctx child_ctx =                                                     \
        ctx_add_alloc(parent_ctx, &list_ctx_node, ALLOC_EXECTREE, tree);
