#pragma once

#include <utils/vec.h>

#include "symtab.h"

#define EXP_DOLLAR (1 << 0)

static char *symexp_word(const char *word)
{
    struct vec expvec;
    vec_init(&expvec);

    int mode = 0;
    int i = 0;
    char key[32];
    char c;
    while ((c = *word++) != '\0')
    {
        if (c == '\'' || c == '"')
            continue;

        if (c == '$')
        {
            i = 0;
            mode |= EXP_DOLLAR;
        }
        else if (mode & EXP_DOLLAR)
        {
            key[i++] = c;
            key[i] = '\0';
            struct kvpair *kv = symtab_lookup(symtab, key);
            if (kv)
            {
                for (char *s = kv->value; *s != '\0'; s++)
                    vec_push(&expvec, *s);
                mode &= ~EXP_DOLLAR;
            }
        }
        else
        {
            vec_push(&expvec, c);
        }
    }

    char *expword = strdup(vec_cstring(&expvec));
    vec_destroy(&expvec);
    return expword;
}
