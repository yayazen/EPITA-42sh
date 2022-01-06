#include <setjmp.h>

#include "builtins.h"
#include "ctx.h"

/** \brief Validate number passed as argument */
int __validate_number(const char *n)
{
    if (!*n)
        return false;

    while (*n)
    {
        if (*n < '0' || *n > '9')
            return false;
        n++;
    }

    return true;
}

/**
 * exit builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/009604499/utilities/exit.html
 */
int bi_exit(const struct ctx *ctx, char **args)
{
    int status = 0;

    // Check if a custom exit status was specified
    if (args[1])
    {
        if (!__validate_number(args[1]))
        {
            fprintf(stderr, "%s: Illegal number: %s\n", args[0], args[1]);
            return 2;
        }

        status = atoi(args[1]);
    }

    *ctx->exit_status = status;

    ctx_free_allocated_memory(ctx, 0);
    longjmp(*ctx->exit_jump, 1);

    return -1;
}