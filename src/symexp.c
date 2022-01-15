
#include "symexp.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/vec.h>

#include "parser.h"
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

    // $RANDOM => random number
    if (!strcmp(key, "RANDOM"))
    {
        // 2^15 please read for an explanation of upperbound limit
        // news://news.epita.fr:119/spf5s2$nus$1@inn-7884769fdd-pjdl8.cri.epita.fr
        sprintf(buff, "%d", rand() % 32768);
        return buff;
    }

    // $UID
    if (!strcmp(key, "UID"))
    {
        sprintf(buff, "%d", getuid());
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
    if (s.mode & EXP_DOLLAR)                                                   \
    {                                                                          \
        s.mode &= ~EXP_DOLLAR;                                                 \
        vec_pushstr(&s.expvec, __search_sym(s.ctx, s.key, s.exp_buff));        \
    }

void symexp_word(const struct ctx *ctx, const char *w, struct list *dest)
{
    struct symexp_state s = {
        .ctx = ctx,
        .word = w,
        .dest = dest,
        .mode = 0,
        .i = 0,
    };
    vec_init(&s.expvec);
    memset(s.key, 0, 100);

    while ((s.c = *s.word++) != '\0')
    {
        /* enter / leave simple-double quote mode */
        if ((!(s.mode & DOUBLE_QUOTE) && s.c == '\'')
            || (!(s.mode & SINGLE_QUOTE) && s.c == '"'))
        {
            s.mode ^= s.c == '\'' ? SINGLE_QUOTE : DOUBLE_QUOTE;
            s.mode |= HAD_A_QUOTE;
            QUIT_DOLLARD_MODE;
        }

        /* Switch to dollar */
        else if (!(s.mode & SINGLE_QUOTE) && s.c == '$'
                 && (!(s.mode & EXP_DOLLAR) || s.i != 0))
        {
            QUIT_DOLLARD_MODE;
            s.i = 0;
            s.mode |= EXP_DOLLAR;
        }

        /* Quit dollar mode & Search for symbol */
        else if (s.mode & EXP_DOLLAR
                 && (s.c == ' ' || s.c == '}' || s.c == ')'))
        {
            QUIT_DOLLARD_MODE;
        }

        /* Command substitution */
        else if (s.mode & EXP_DOLLAR && s.i == 0 && s.c == '('
                 && *s.word != ')')
        {
            // Parser section
            struct rl_state ps = RL_DEFAULT_STATE;

            // Allocate memory
            ps.cs = cstream_string_create(s.word - 1);
            vec_init(&ps.word);
            vec_init(&ps.buffered_word);

            if (rl_subshell(&ps) == true)
            {
                int pipefd[2];
                assert(pipe(pipefd) == 0);

                // Redirect stdout before execution
                fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
                int savefd = dup(STDOUT_FILENO);
                fcntl(savefd, F_SETFD, FD_CLOEXEC);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                rl_exec_subshell(ctx, ps.node);

                // Rollback redirect
                dup2(savefd, STDOUT_FILENO);
                close(savefd);

                struct cstream *cs =
                    cstream_file_create(fdopen(pipefd[0], "r"), true);

                int c;
                bool reset = false;
                while (cstream_pop(cs, &c) == NO_ERROR && c != EOF)
                {
                    if (c == ' ' || c == '\n')
                    {
                        reset = true;
                        continue;
                    }

                    if (reset && s.expvec.size > 0)
                    {
                        if (s.mode & DOUBLE_QUOTE)
                            vec_push(&s.expvec, ' ');
                        else
                        {
                            list_push(dest, strdup(vec_cstring(&s.expvec)));
                            vec_reset(&s.expvec);
                        }
                    }
                    reset = false;

                    vec_push(&s.expvec, c);
                }

                cstream_free(cs);
            }
            else
            {
                fprintf(stderr, PACKAGE " : rule mismatch or unimplemented");
            }

            s.word = cstream_string_str(ps.cs) - 1;
            if (*s.word == ')' && *(s.word + 1) == '\0')
                s.word++;

            // Free parser
            cstream_free(ps.cs);
            rl_exectree_free(ps.node);
            vec_destroy(&ps.word);
            vec_destroy(&ps.buffered_word);

            QUIT_DOLLARD_MODE
        }

        /* Escape characters */
        else if (s.c == '\\' && *s.word)
        {
            QUIT_DOLLARD_MODE
            if (*s.word == '\\'
                || (!(s.mode & DOUBLE_QUOTE) && !(s.mode & SINGLE_QUOTE))
                || (s.mode & DOUBLE_QUOTE
                    && (*s.word == '"' || *s.word == '`' || *s.word == '\n')))
            {
                if (*s.word != '\n')
                {
                    vec_push(&s.expvec, *s.word);
                }
                s.word++;
            }
            else
            {
                vec_push(&s.expvec, '\\');
            }
        }

        /* In symbol acquisition */
        else if (!(s.mode & SINGLE_QUOTE) && (s.mode & EXP_DOLLAR))
        {
            if (s.c == '(' || s.c == '{')
                continue;

            // special case : shell arguments
            if (__is_int(s.key) && !__is_digit(s.c))
            {
                s.word--;
                QUIT_DOLLARD_MODE
            }

            // default case
            else
            {
                s.key[s.i++] = s.c;
                s.key[s.i] = '\0';
            }

            // Special variable with a single character
            struct __single_char_args a = { .c = s.c, .mode = s.mode };
            if (s.i == 1 && __exp_single_char(ctx, dest, &s.expvec, &a))
            {
                s.mode &= ~EXP_DOLLAR;
            }
        }

        else
        {
            vec_push(&s.expvec, s.c);
        }
    }

    QUIT_DOLLARD_MODE;

    if (s.expvec.size > 0 || s.mode & HAD_A_QUOTE)
        list_push(dest, strdup(vec_cstring(&s.expvec)));

    vec_destroy(&s.expvec);
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