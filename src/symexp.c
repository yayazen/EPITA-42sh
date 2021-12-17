#include "symexp.h"

#include <stdlib.h>
#include <utils/vec.h>

#define EXP_DOLLAR (1 << 0)
#define SINGLE_QUOTE (1 << 2)
#define DOUBLE_QUOTE (1 << 3)

/**
 * \brief Search for a symbol in symbol table & environment variables
 * \param symtab A pointer on the symbol table to use
 * \param key The key to search
 */
static char *__search_sym(struct symtab *symtab, const char *key)
{
    struct kvpair *kv = symtab_lookup(symtab, key);
    if (kv && kv->type == KV_WORD)
    {
        return kv->value.word;
    }

    return getenv(key);
}

char *symexp_word(struct symtab *symtab, const char *word)
{
    struct vec expvec;
    vec_init(&expvec);

    int mode = 0;
    int i = 0;
    char key[32];
    char c;
    while ((c = *word++) != '\0')
    {
        if (!(mode & DOUBLE_QUOTE) && c == '\'')
        {
            mode ^= SINGLE_QUOTE;
            continue;
        }

        else if (!(mode & SINGLE_QUOTE) && c == '"')
        {
            printf("toggle dl\n");
            mode ^= DOUBLE_QUOTE;
            continue;
        }

        else if (!(mode & SINGLE_QUOTE) && c == '$')
        {
            i = 0;
            mode |= EXP_DOLLAR;
        }

        else if (!(mode & SINGLE_QUOTE) && (mode & EXP_DOLLAR))
        {
            key[i++] = c;
            key[i] = '\0';

            char *sym = __search_sym(symtab, key);
            if (sym != NULL)
            {
                for (char *s = sym; *s != '\0'; s++)
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
