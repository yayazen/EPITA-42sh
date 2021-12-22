#include <setjmp.h>

#include "builtins.h"
#include "ctx.h"
#include "list.h"

/** \brief jump to the appropriate loop */
int __stop_loop(int val, const struct ctx *ctx, char **args)
{
    // Get stop level
    int level = args[1] && *args[1] ? atoi(args[1]) : 1;
    if (level < 1)
    {
        fprintf(stderr, "%s: illegal number %s\n", args[0], args[1]);
        return 2;
    }

    struct ctx_jmp *jmp = ctx->loop_jump;
    while (jmp && jmp->next && level > 1)
    {
        jmp = jmp->next;
        level--;
    }

    if (jmp)
    {
        ctx_free_allocated_memory(ctx, jmp->level);
        longjmp(*jmp->jump, val);
    }

    return 0;
}

/**
 * continue builtin
 *
 * \ref
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#continue
 */
int bi_continue(const struct ctx *ctx, char **args)
{
    return __stop_loop(JMP_CONTINUE, ctx, args);
}

/**
 * break builtin
 *
 * \ref
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#break
 */
int bi_break(const struct ctx *ctx, char **args)
{
    return __stop_loop(JMP_BREAK, ctx, args);
}