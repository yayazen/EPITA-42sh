#include <assert.h>

#include "builtins.h"
#include "ctx.h"
#include "symtab.h"

/**
 * export builtin
 *
 * \ref
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_22
 */
int bi_export(const struct ctx *ctx, char **args)
{
    assert(ctx && args);

    // By default, we print symtab (SCL does not give any instruction about how
    // to handle export without argument)
    if (!args[1])
    {
        symtab_print(ctx->st);
        return 0;
    }

    char **seek = args + 1;
    while (*seek)
    {
        char *eq_pos = strchr(*seek, '=');

        // Case 1. => there is no equal sign
        if (!eq_pos)
        {
            struct kvpair *pair = symtab_lookup(ctx->st, *seek, KV_WORD);

            // Create a pair if required
            if (!pair)
                pair = symtab_add(ctx->st, *seek, KV_WORD, strdup(""));

            // Mark the value as exported
            pair->value.word.exported = true;
        }

        // Case 2. => there is an equal sign
        else
        {
            char *key = strdup(*seek);
            *(key + (eq_pos - *seek)) = '\0';
            symtab_add(ctx->st, key, KV_WORD, strdup(eq_pos + 1))
                ->value.word.exported = true;
            free(key);
        }

        seek++;
    }

    return 0;
}