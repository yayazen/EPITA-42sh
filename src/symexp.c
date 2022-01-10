#include "symexp.h"

#include <stdlib.h>
#include <utils/vec.h>

#include "symtab.h"

#define EXP_DOLLAR (1 << 0)
#define SINGLE_QUOTE (1 << 2)
#define DOUBLE_QUOTE (1 << 3)
#define HAD_A_QUOTE (1 << 4)

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

    return NULL;
}

void symexp_word(const struct ctx *ctx, const char *word, struct list *dest)
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
            mode |= HAD_A_QUOTE;
        }

        /* enter / leave double quote mode */
        else if (!(mode & SINGLE_QUOTE) && c == '"')
        {
            mode ^= DOUBLE_QUOTE;
            mode |= HAD_A_QUOTE;
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

            char *sym = __search_sym(ctx->st, key);
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

    if (expvec.size > 0 || mode & HAD_A_QUOTE)
        list_push(dest, strdup(vec_cstring(&expvec)));

    vec_destroy(&expvec);
}

char *symexp_word_single_result(const struct ctx *ctx, const char *word)
{
    struct list *l = list_new(1);

    // Perform expansion
    symexp_word(ctx, word, l);
    char *res = strdup(l->size == 0 ? "" : l->data[0]);

    list_free(l);

    return res;
}