#include <assert.h>

#include "builtins.h"
#include "constants.h"
#include "ctx.h"
#include "io/cstream.h"
#include "parser.h"

static void __exit(const struct ctx *ctx, int status)
{
    *ctx->exit_status = status;
    ctx_free_allocated_memory(ctx, 0);
    longjmp(*ctx->exit_jump,
            ctx->is_interactive ? EXIT_WITHOUT_LOOP_EXIT : EXIT_WITH_LOOP_EXIT);
}

/**
 * source builtin
 *
 * \ref
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_18
 */
int bi_source(const struct ctx *ctx, char **args)
{
    assert(ctx && args);

    if (!args[1])
        return 0;

    // TODO : resolve PATH
    char *path = args[1];

    // If file could not be opened, go back to parser loop
    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, "%s: could not open %s!\n", args[0], args[1]);
        __exit(ctx, 127);
    }

    // Execute commands
    struct cstream *cs = cstream_file_create(file, true);
    int err;
    while ((err = parser(cs, 0, ctx->exit_status, ctx->st)) == NO_ERROR)
    {
        ;
    }
    cstream_free(cs);

    // Check for parsing error
    if (err != REACHED_EOF)
    {
        __exit(ctx, 2);
    }

    return *ctx->exit_status;
}