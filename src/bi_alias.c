#include <assert.h>

#include "builtins.h"

/** \brief Print a single alias entry */
static void __print_alias(struct kvpair *pair)
{
    printf("%s=%s\n", pair->key, pair->value.word);
}

/** \brief Show the entire alias available in symtab */
static void __print_all_aliases(struct symtab *st)
{
    for (size_t i = 0; i < st->capacity; i++)
    {
        struct kvpair *kv = st->data[i];
        while (kv)
        {
            if (kv->type == KV_ALIAS)
                __print_alias(kv);
            kv = kv->next;
        }
    }
}

/**
 * alias builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/9699919799/utilities/alias.html
 */
int bi_alias(char **args, struct symtab *s)
{
    assert(args && s);

    /* skip builtin name */
    args++;

    /* Print aliases list */
    if (*args == NULL)
    {
        __print_all_aliases(s);
        return 0;
    }

    /* Process alias list */
    int ret = 0;
    while (*args != NULL)
    {
        char *eqpos = strchr(*args, '=');

        /* Request to display alias */
        if (eqpos == NULL)
        {
            struct kvpair *pair = symtab_lookup(s, *args);
            if (!pair || pair->type != KV_ALIAS)
                ret = 1;
            else
                __print_alias(pair);
        }

        /* Request to set a new alias */
        else
        {
            char *key = strdup(*args);
            key[eqpos - *args] = '\0';
            symtab_add(s, key, KV_ALIAS, strdup(eqpos + 1));
            free(key);
        }

        args++;
    }

    fflush(stdout);
    return ret;
}