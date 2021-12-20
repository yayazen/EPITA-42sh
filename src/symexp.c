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
    struct kvpair *kv = symtab_lookup(symtab, key, KV_WORD);
    if (kv && kv->type == KV_WORD)
    {
        return kv->value.word.word;
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
        /* enter / leave simple quote mode */
        if (!(mode & DOUBLE_QUOTE) && c == '\'')
        {
            mode ^= SINGLE_QUOTE;
        }

        /* enter / leave double quote mode */
        else if (!(mode & SINGLE_QUOTE) && c == '"')
        {
            mode ^= DOUBLE_QUOTE;
        }

        /* Switch to dolar */
        else if (!(mode & SINGLE_QUOTE) && c == '$')
        {
            i = 0;
            mode |= EXP_DOLLAR;
        }

        /* Quit dolar mode */
        else if (mode & EXP_DOLLAR && (c == ' ' || c == '}' || c == ')'))
        {
            mode &= ~EXP_DOLLAR;
            // vec_push(&expvec, c);
        }

        /* Search for symbol */
        else if (!(mode & SINGLE_QUOTE) && (mode & EXP_DOLLAR))
        {
            if (c == '(' || c == '{')
                continue;
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
