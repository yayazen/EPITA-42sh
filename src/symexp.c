/**
 * \file symexp.c
 * \brief Symbols expansion implementation.
 *
 * This file contains a lot of dirty code. I was really tired when I wrote it,
 * so I just made sure my code was working. I did not care of the rest. Sorry.
 */

#include "symexp.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/strings.h>
#include <utils/vec.h>

#include "parser.h"
#include "symtab.h"

#define EXP_DOLLAR (1 << 0)
#define SINGLE_QUOTE (1 << 2)
#define DOUBLE_QUOTE (1 << 3)
#define HAD_A_QUOTE (1 << 4)

/**
 * \brief Search for a symbol in symbol table & environment variables
 * \param ctx Execution context
 * \param key The key to search
 */
static char *__search_sym(struct symexp_state *s)
{
    // Check in program arguments
    if (is_int(s->key))
    {
        int index = atoi(s->key);
        if (index < s->ctx->program_args_count)
        {
            return s->ctx->program_args[index];
        }
    }

    // $RANDOM => random number
    if (!strcmp(s->key, "RANDOM"))
    {
        // 2^15 please read for an explanation of upperbound limit
        // news://news.epita.fr:119/spf5s2$nus$1@inn-7884769fdd-pjdl8.cri.epita.fr
        sprintf(s->exp_buff, "%d", rand() % 32768);
        return s->exp_buff;
    }

    // $UID
    if (!strcmp(s->key, "UID"))
    {
        sprintf(s->exp_buff, "%d", getuid());
        return s->exp_buff;
    }

    // Search in symbols table
    struct kvpair *kv = symtab_lookup(s->ctx->st, s->key, KV_WORD);
    if (kv && kv->type == KV_WORD)
    {
        return kv->value.word.word;
    }

    return NULL;
}

/** \brief Perform $@ expansion */
static void __exp_args_array(struct symexp_state *s)
{
    if (s->ctx->program_args_count < 2)
        return;

    // Handle first arguments
    vec_pushstr(&s->expvec, s->ctx->program_args[1]);

    if (s->ctx->program_args_count == 2)
        return;

    list_push(s->dest, strdup(vec_cstring(&s->expvec)));
    vec_reset(&s->expvec);

    // Handles arguments "in the middle"
    for (int i = 2; i < s->ctx->program_args_count - 1; i++)
        list_push(s->dest, strdup(s->ctx->program_args[i]));

    // Handle last argument
    vec_pushstr(&s->expvec,
                s->ctx->program_args[s->ctx->program_args_count - 1]);
}

/** \brief expand special variable with a single character */
static int __exp_single_char(struct symexp_state *s)
{
    char buff[10];

    switch (s->c)
    {
    // $@ => program arguments as an array
    case '@':
        __exp_args_array(s);
        return true;

    // $* => program arguments as a simple list
    case '*':
        if (s->mode & DOUBLE_QUOTE)
            for (int i = 1; i < s->ctx->program_args_count; i++)
            {
                if (i > 1)
                    vec_push(&s->expvec, ' ');
                vec_pushstr(&s->expvec, s->ctx->program_args[i]);
            }
        else
            __exp_args_array(s);
        return true;

    // $? => last exit status
    case '?':
        sprintf(buff, "%d", *s->ctx->exit_status);
        vec_pushstr(&s->expvec, buff);
        return true;

    // $$ => current process id
    case '$':
        sprintf(buff, "%d", getpid());
        vec_pushstr(&s->expvec, buff);
        return true;

    // $# => number of arguments
    case '#':
        sprintf(buff, "%d",
                !(s->ctx->flags & IN_FUNCTION)
                        && (s->ctx->flags & MODE_INPUT
                            || s->ctx->program_args_count == 0)
                    ? s->ctx->program_args_count
                    : s->ctx->program_args_count - 1);
        vec_pushstr(&s->expvec, buff);
        return true;

    default:
        return false;
    }
}

void quit_dollard_mode(struct symexp_state *s)
{
    if (s->mode & EXP_DOLLAR)
    {
        s->mode &= ~EXP_DOLLAR;

        if (s->i == 0)
            vec_push(&s->expvec, '$');

        else
            vec_pushstr(&s->expvec, __search_sym(s));
    }
}

/** \brief turn "`...`" into "(...)" */
static char *__convert_substitution(const char *input_str)
{
    char *temp_str = strdup(input_str);
    temp_str[0] = '(';
    char *end = strchr(temp_str + 1, '`');
    if (end)
        *end = ')';
    return temp_str;
}

/** \brief read a stream and split its content into a list */
static void __read_stream_into_list(int fd, struct vec *expvec, int mode,
                                    struct list *dest)
{
    struct cstream *cs = cstream_file_create(fdopen(fd, "r"), true);
    int c;
    bool reset = false;
    while (cstream_pop(cs, &c) == NO_ERROR && c != EOF)
    {
        if (c == ' ' || c == '\n')
        {
            reset = true;
            continue;
        }

        if (reset && expvec->size > 0)
        {
            if (mode & DOUBLE_QUOTE)
                vec_push(expvec, ' ');
            else
            {
                list_push(dest, strdup(vec_cstring(expvec)));
                vec_reset(expvec);
            }
        }
        reset = false;

        vec_push(expvec, c);
    }

    cstream_free(cs);
}

/** \brief perform command subsitution expansion */
static void __exp_cmd_substitution(struct symexp_state *s)
{
    // Parser section
    struct rl_state ps = RL_DEFAULT_STATE;

    // Allocate memory
    const char *input_str = s->word - 1;
    char *temp_str = NULL;

    // Handle the case of '`' cmd '`'
    if (*input_str == '`')
    {
        temp_str = __convert_substitution(input_str);
        input_str = temp_str;
    }

    // Initialize streams
    ps.cs = cstream_string_create(input_str);
    vec_init(&ps.word);
    vec_init(&ps.buffered_word);
    ps.flag |= LEX_CMDSTART;

    // Check if subshell is empty
    if (*ltrim(input_str + 1) == ')')
    {
        int c;
        while (cstream_pop(ps.cs, &c) == NO_ERROR && c != ')')
            ;
        goto after_exec_substitution;
    }

    // Attempt to parse substring. In case of success, execute it in subshell
    // environment
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

        rl_exec_subshell(s->ctx, ps.node);

        // Rollback redirect
        dup2(savefd, STDOUT_FILENO);
        close(savefd);

        __read_stream_into_list(pipefd[0], &s->expvec, s->mode, s->dest);
    }
    else
    {
        fprintf(stderr, PACKAGE " : rule mismatch or unimplemented");
    }

after_exec_substitution:

    s->word = s->word + (cstream_string_str(ps.cs) - input_str - 2);
    if ((*s->word == ')' || *s->word == '`') && *(s->word + 1) == '\0')
        s->word++;

    // Free parser
    cstream_free(ps.cs);
    rl_exectree_free(ps.node);
    vec_destroy(&ps.word);
    vec_destroy(&ps.buffered_word);

    if (temp_str)
        free(temp_str);

    s->mode &= ~EXP_DOLLAR;
}

/** \brief expand escape character */
static void __expand_escape_character(struct symexp_state *s)
{
    quit_dollard_mode(s);
    if (*s->word == '\\'
        || (!(s->mode & DOUBLE_QUOTE) && !(s->mode & SINGLE_QUOTE))
        || (s->mode & DOUBLE_QUOTE
            && (*s->word == '"' || *s->word == '`' || *s->word == '\n')))
    {
        if (*s->word != '\n')
        {
            vec_push(&s->expvec, *s->word);
        }
        s->word++;
    }
    else
    {
        vec_push(&s->expvec, '\\');
    }
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
            quit_dollard_mode(&s);
        }

        /* Switch to dollar */
        else if (!(s.mode & SINGLE_QUOTE) && s.c == '$'
                 && (!(s.mode & EXP_DOLLAR) || s.i != 0))
        {
            quit_dollard_mode(&s);
            s.i = 0;
            s.mode |= EXP_DOLLAR;
        }

        /* Quit dollar mode & Search for symbol */
        else if (s.mode & EXP_DOLLAR
                 && (s.c == ' ' || s.c == '}' || s.c == ')'))
        {
            quit_dollard_mode(&s);
        }

        /* Command substitution '$(' cmd ')' */
        else if (s.mode & EXP_DOLLAR && s.i == 0 && s.c == '(')
        {
            __exp_cmd_substitution(&s);
        }

        /* Command subsitution '`' cdm '`' */
        else if (!(s.mode & SINGLE_QUOTE) && s.c == '`')
        {
            quit_dollard_mode(&s);
            __exp_cmd_substitution(&s);
        }

        /* Escape characters */
        else if (s.c == '\\' && *s.word)
        {
            __expand_escape_character(&s);
        }

        /* In symbol acquisition */
        else if (!(s.mode & SINGLE_QUOTE) && (s.mode & EXP_DOLLAR))
        {
            if (s.c == '(' || s.c == '{')
                continue;

            // special case : shell arguments
            if (is_int(s.key) && !is_digit(s.c))
            {
                s.word--;
                quit_dollard_mode(&s);
            }

            // default case
            else
            {
                s.key[s.i++] = s.c;
                s.key[s.i] = '\0';
            }

            // Special variable with a single character
            if (s.i == 1 && __exp_single_char(&s))
            {
                s.mode &= ~EXP_DOLLAR;
            }
        }

        else
        {
            vec_push(&s.expvec, s.c);
        }
    }

    quit_dollard_mode(&s);

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