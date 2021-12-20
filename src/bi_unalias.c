#include <assert.h>

#include "builtins.h"

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
int bi_unalias(char **args, struct symtab *s)
{
    int ret = 0;
    assert(args && s);

    args++;

    /* delete all aliases */
    if (*args && !strcmp(*args, "-a"))
    {
        __delete_all_aliases(s);
        return 0;
    }

    while (*args)
    {
        struct kvpair *pair = symtab_lookup(s, *args, KV_ALIAS);

        /* the symbol is invalid / not found */
        if (!pair || pair->type != KV_ALIAS)
        {
            fprintf(stderr, "unalias: %s not found\n", *args);
            ret = 1;
        }

        else
        {
            symtab_del(s, pair);
        }

        args++;
    }

    return ret;
}