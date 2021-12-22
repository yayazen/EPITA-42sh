#include <setjmp.h>

#include "builtins.h"
#include "ctx.h"

/**
 * exit builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/009604499/utilities/exit.html
 */
int bi_exit(const struct ctx *ctx, char **args)
{
    int status = 0;

    if (args[1] && *args[1])
        status = atoi(args[1]);

    *ctx->exit_status = status;

    ctx_free_allocated_memory(ctx, 0);
    longjmp(*ctx->exit_jump, 1);

    return -1;
}