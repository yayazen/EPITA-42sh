#include "symexp.h"

#include <stdlib.h>
#include <utils/vec.h>

#include "symtab.h"

#define EXP_DOLLAR (1 << 0)
#define SINGLE_QUOTE (1 << 2)
#define DOUBLE_QUOTE (1 << 3)
#define HAD_A_QUOTE (1 << 4)

#define __is_digit(c) (c >= '0' && c <= '9')

struct __single_char_args
{
    char c;
    int mode;
};

/** \brief Check if a given string is an integer or not */
static bool __is_int(const char *val)
{
    if (!val || !*val)
        return false;
    while (*val && __is_digit(*val))
        val++;
    return *val == '\0';
}

/**
 * \brief Search for a symbol in symbol table & environment variables
 * \param ctx Execution context
 * \param key The key to search
 */
static char *__search_sym(const struct ctx *ctx, const char *key, char *buff)
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

    // $RANDOM
    if (!strcmp(key, "RANDOM"))
    {
        // 2^15 please read for an explanation of upperbound limit
        // news://news.epita.fr:119/spf5s2$nus$1@inn-7884769fdd-pjdl8.cri.epita.fr
        sprintf(buff, "%d", rand() % 32768);
        return buff;
    }

    // Search in symbols table
    struct kvpair *kv = symtab_lookup(ctx->st, key, KV_WORD);
    if (kv && kv->type == KV_WORD)
    {
        return kv->value.word.word;
    }

    return NULL;
}

/** \brief Perform $@ expansion */
static void __exp_args_array(const struct ctx *ctx, struct list *dest,
                             struct vec *vec)
{
    if (ctx->program_args_count < 2)
        return;

    // Handle first arguments
    vec_pushstr(vec, ctx->program_args[1]);

    if (ctx->program_args_count == 2)
        return;

    list_push(dest, strdup(vec_cstring(vec)));
    vec_reset(vec);

    // Handles arguments "in the middle"
    for (int i = 2; i < ctx->program_args_count - 1; i++)
        list_push(dest, strdup(ctx->program_args[i]));

    // Handle last argument
    vec_pushstr(vec, ctx->program_args[ctx->program_args_count - 1]);
}

/** \brief expand special variable with a single character */
static int __exp_single_char(const struct ctx *ctx, struct list *dest,
                             struct vec *vec, struct __single_char_args *a)
{
    char buff[10];

    switch (a->c)
    {
    // $@ => program arguments as an array
    case '@':
        __exp_args_array(ctx, dest, vec);
        return true;

    // $* => program arguments as a simple list
    case '*':
        if (a->mode & DOUBLE_QUOTE)
            for (int i = 1; i < ctx->program_args_count; i++)
            {
                if (i > 1)
                    vec_push(vec, ' ');
                vec_pushstr(vec, ctx->program_args[i]);
            }
        else
            __exp_args_array(ctx, dest, vec);
        return true;

    // $? => last exit status
    case '?':
        sprintf(buff, "%d", *ctx->exit_status);
        vec_pushstr(vec, buff);
        return true;

    // $$ => current process id
    case '$':
        sprintf(buff, "%d", getpid());
        vec_pushstr(vec, buff);
        return true;

    // $# => number of arguments
    case '#':
        sprintf(buff, "%d",
                ctx->flags & MODE_INPUT || ctx->program_args_count == 0
                    ? ctx->program_args_count
                    : ctx->program_args_count - 1);
        vec_pushstr(vec, buff);
        return true;

    default:
        return false;
    }
}

#define QUIT_DOLLARD_MODE                                                      \
    if (mode & EXP_DOLLAR)                                                     \
    {                                                                          \
        mode &= ~EXP_DOLLAR;                                                   \
        vec_pushstr(&expvec, __search_sym(ctx, key, exp_buff));                \
    }

void symexp_word(const struct ctx *ctx, const char *word, struct list *dest)
{
    struct vec expvec;
    vec_init(&expvec);

    int mode = 0;
    int i = 0;
    char key[100] = { '\0' };
    char exp_buff[10]; // some special variables needs to write somewhere
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

        /* Switch to dollar */
        else if (!(mode & SINGLE_QUOTE) && c == '$'
                 && (!(mode & EXP_DOLLAR) || i != 0))
        {
            QUIT_DOLLARD_MODE;
            i = 0;
            mode |= EXP_DOLLAR;
        }

        /* Quit dollar mode & Search for symbol */
        else if (mode & EXP_DOLLAR && (c == ' ' || c == '}' || c == ')'))
        {
            QUIT_DOLLARD_MODE;
        }

        /* In symbol acquisition */
        else if (!(mode & SINGLE_QUOTE) && (mode & EXP_DOLLAR))
        {
            if (c == '(' || c == '{')
                continue;

            // special case : shell arguments
            if (__is_int(key) && !__is_digit(c))
            {
                word--;
                QUIT_DOLLARD_MODE
            }

            // default case
            else
            {
                key[i++] = c;
                key[i] = '\0';
            }

            // Special variable with a single character
            struct __single_char_args a = { .c = c, .mode = mode };
            if (i == 1 && __exp_single_char(ctx, dest, &expvec, &a))
            {
                mode &= ~EXP_DOLLAR;
            }
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