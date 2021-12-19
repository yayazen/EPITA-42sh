#include <assert.h>

#include "builtins.h"

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
        fprintf(stderr, "TODO : print aliases list;\n");
        return 1;
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
                printf("%s=%s\n", pair->key, pair->value.word);
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