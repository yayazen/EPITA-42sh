#include "symexp.h"

#include <stdlib.h>
#include <utils/vec.h>

#include "symtab.h"

#define EXP_DOLLAR (1 << 0)
#define SINGLE_QUOTE (1 << 2)
#define DOUBLE_QUOTE (1 << 3)
#define HAD_A_QUOTE (1 << 4)

/** \brief Check if a given string is an integer or not */
static bool __is_int(const char *val)
{
    if (!val || !*val)
        return false;
    while (*val && *val >= '0' && *val <= '9')
        val++;
    return *val == '\0';
}

/**
 * \brief Search for a symbol in symbol table & environment variables
 * \param ctx Execution context
 * \param key The key to search
 */
static char *__search_sym(const struct ctx *ctx, const char *key)
{
    // Check in program arguments
    if (__is_int(key))
    {
        int index = atoi(key);
        if (index < ctx->program_args_count)
        {
            return ctx->program_args[index];
        }
    }

    // Search in symbols table
    struct kvpair *kv = symtab_lookup(ctx->st, key, KV_WORD);
    if (kv && kv->type == KV_WORD)
    {
        return kv->value.word.word;
    }

    return NULL;
}

#define QUIT_DOLLARD_MODE                                                      \
    if (mode & EXP_DOLLAR)                                                     \
    {                                                                          \
        mode &= ~EXP_DOLLAR;                                                   \
        vec_pushstr(&expvec, __search_sym(ctx, key));                          \
    }

void symexp_word(const struct ctx *ctx, const char *word, struct list *dest)
{
    struct vec expvec;
    vec_init(&expvec);

    int mode = 0;
    int i = 0;
    char key[100];
    char c;
    while ((c = *word++) != '\0')
    {
        /* enter / leave simple quote mode */
        if (!(mode & DOUBLE_QUOTE) && c == '\'')
        {
            mode ^= SINGLE_QUOTE;
            mode |= HAD_A_QUOTE;
            QUIT_DOLLARD_MODE;
        }

        /* enter / leave double quote mode */
        else if (!(mode & SINGLE_QUOTE) && c == '"')
        {
            mode ^= DOUBLE_QUOTE;
            mode |= HAD_A_QUOTE;
            QUIT_DOLLARD_MODE;
        }

        /* Switch to dolar */
        else if (!(mode & SINGLE_QUOTE) && c == '$')
        {
            QUIT_DOLLARD_MODE;
            i = 0;
            mode |= EXP_DOLLAR;
        }

        /* Quit dolar mode & Search for symbol */
        else if (mode & EXP_DOLLAR && (c == ' ' || c == '}' || c == ')'))
        {
            QUIT_DOLLARD_MODE;
        }

        /* In symbol acqusition */
        else if (!(mode & SINGLE_QUOTE) && (mode & EXP_DOLLAR))
        {
            if (c == '(' || c == '{')
                continue;
            key[i++] = c;
            key[i] = '\0';
        }

        else
        {
            vec_push(&expvec, c);
        }
    }

    QUIT_DOLLARD_MODE;

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