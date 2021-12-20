#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "builtins.h"

enum
{
    DEFAULT_MODE = 0,
    WORD = (1 << 1),
    FUNC = (1 << 2),
};

/** \brief Parse arguments passed to function */
char **__parse_args(char **args, int *mode, int *ret)
{
    while (*args && **args == '-')
    {
        char *seek = (*args) + 1;

        if (!*seek)
        {
            fprintf(stderr, "unset: invalid variable name: -");
            *ret = 2;
            return NULL;
        }

        while (*seek)
        {
            switch (*seek)
            {
            case 'f':
                *mode |= FUNC;
                break;

            case 'v':
                *mode |= WORD;
                break;

            default:
                fprintf(stderr, "unset: illegal option -%c", *seek);
                *ret = 2;
                return NULL;
            }

            seek++;
        }

        args++;
    }

    return args;
}

/**
 * unset builtin
 *
 * \ref
 * https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset
 */
int bi_unset(char **args, struct symtab *s)
{
    assert(args && s);
    args++;
    int ret = 0;
    int mode = DEFAULT_MODE;
    args = __parse_args(args, &mode, &ret);

    while (args && *args)
    {
        /* word */
        if (mode == DEFAULT_MODE || mode & WORD)
        {
            struct kvpair *p = symtab_lookup(s, *args, KV_WORD);
            if (p && p->type == KV_WORD)
                symtab_del(s, p);
        }

        /* function */
        if (mode & FUNC)
        {
            struct kvpair *p = symtab_lookup(s, *args, KV_FUNC);
            if (p && p->type == KV_FUNC)
                symtab_del(s, p);
        }

        args++;
    }

    return ret;
}