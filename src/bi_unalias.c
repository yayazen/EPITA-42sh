#include <assert.h>

#include "builtins.h"
#include "ctx.h"

/** \brief Delete all aliases */
void __delete_all_aliases(struct symtab *st)
{
    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        while (kv)
        {
            struct kvpair *next = kv->next;
            if (kv->type == KV_ALIAS)
                symtab_del(st, kv);
            kv = next;
        }
    }
}

/**
 * unalias builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/9699919799/utilities/unalias.html
 */
int bi_unalias(char **args, const struct ctx *ctx)
{
    int ret = 0;
    assert(args && ctx);

    args++;

    /* delete all aliases */
    if (*args && !strcmp(*args, "-a"))
    {
        __delete_all_aliases(ctx->st);
        return 0;
    }

    while (*args)
    {
        struct kvpair *pair = symtab_lookup(ctx->st, *args, KV_ALIAS);

        /* the symbol is invalid / not found */
        if (!pair || pair->type != KV_ALIAS)
        {
            fprintf(stderr, "unalias: %s not found\n", *args);
            ret = 1;
        }

        else
        {
            symtab_del(ctx->st, pair);
        }

        args++;
    }

    return ret;
}