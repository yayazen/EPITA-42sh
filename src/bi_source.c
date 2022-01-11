#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <linux/limits.h>

#include "builtins.h"
#include "constants.h"
#include "ctx.h"
#include "io/cstream.h"
#include "parser.h"

/** \brief exit from script, returning a specific exit status */
static void __exit(const struct ctx *ctx, int status)
{
    *ctx->exit_status = status;
    ctx_free_allocated_memory(ctx, 0);
    longjmp(*ctx->exit_jump,
            ctx->is_interactive ? EXIT_WITHOUT_LOOP_EXIT : EXIT_WITH_LOOP_EXIT);
}

/** \brief search for a file in PATH */
static char *__search_in_path(const struct ctx *ctx, const char *file)
{
    assert(ctx && file);

    struct kvpair *path_pair = symtab_lookup(ctx->st, "PATH", KV_WORD);

    if (path_pair == NULL)
        return NULL;

    char *saveptr = NULL;
    char *pos = NULL;
    char *path_str = strdup(path_pair->value.word.word);
    char *found_path = NULL;

    while ((pos = strtok_r(saveptr == NULL ? path_str : NULL, ":", &saveptr)))
    {
        char buff[PATH_MAX + 1];
        snprintf(buff, PATH_MAX, "%s/%s", pos, file);

        if (access(buff, F_OK) == 0)
        {
            found_path = strdup(buff);
            break;
        }
    }

    free(path_str);
    return found_path;
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

    // If second argument does not contain a '/', search for appropriate file in
    // path
    char *path = strchr(args[1], '/') == NULL ? __search_in_path(ctx, args[1])
                                              : strdup(args[1]);
    if (path == NULL)
    {
        fprintf(stderr, "%s: could not find %s!\n", args[0], args[1]);
        free(path);
        __exit(ctx, 2);
    }

    // If file could not be opened, go back to parser loop
    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, "%s: could not open %s!\n", args[0], path);
        free(path);
        __exit(ctx, 127);
    }
    free(path);

    // Execute commands
    struct cstream *cs = cstream_file_create(file, true);
    int err;
    struct parser_args parser_args = {
        .cs = cs,
        .flag = 0,
        .exit_status = ctx->exit_status,
        .symtab = ctx->st,
        .program_args = ctx->program_args,
        .program_args_count = ctx->program_args_count,
        .running_script = ctx->running_script,
    };
    while ((err = parser(&parser_args)) == NO_ERROR)
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