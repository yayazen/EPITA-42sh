#include "ast/lexer.h"

#include "ast/token.h"
#include "err.h"
#include "io/cstream.h"

/*
 * @brief   return pointer to next non-blank char in `s`
 */
static inline const char *__eat_whitespaces(const char *s)
{
    for (; *s == '\t' || *s == ' '; s++)
        ;
    return s;
}

/*
 * @brief   return success if `p` is a prefix of `s`
 */
static inline int __is_prefix(const char *p, const char *s)
{
    for (; *p != '\0' && *s == *p; s++, p++)
        ;
    return *p == '\0';
}

/*
 * @brief   return the longest token that is a prefix of `s`
 */
static inline int __longest_prefix_token(const char *s)
{
    int m = 0;
    for (int n = 1; n < TOKEN_COUNT; n++)
    {
        if (TOKEN_STRLEN(n) > TOKEN_STRLEN(m) && __is_prefix(TOKEN_STR(n), s))
            m = n;
    }
    return m;
}

static inline int __pop_stream(struct cstream *cs)
{
    int c;
    if (cstream_pop(cs, &c) != NO_ERROR)
    {
        warn("Failed to read stream!");
        abort();
    }
    return c;
}

static inline int __peek_stream(struct cstream *cs)
{
    int c;
    if (cstream_peek(cs, &c) != NO_ERROR)
    {
        warn("Failed to read stream!");
        abort();
    }
    return c;
}

static inline int __interrupting_char(char c)
{
    return strchr("\n<>;|&!(){} \t\r`", c) != NULL;
}

enum READMODE
{
    NO_QUOTE,
    SIMPLE_QUOTE,
    DOUBLE_QUOTE
};

static void __collect_next_token(struct state *s)
{
    enum READMODE mode = NO_QUOTE;

    if (__peek_stream(s->cs) == '"' || __peek_stream(s->cs) == '\'')
    {
        mode = __peek_stream(s->cs) == '"' ? DOUBLE_QUOTE : SIMPLE_QUOTE;
        __pop_stream(s->cs);
    }

    if (__interrupting_char(__peek_stream(s->cs)))
    {
        vec_push(&s->last_token_str, __pop_stream(s->cs));
        return;
    }

    while (__peek_stream(s->cs) != EOF)
    {
        char n = __peek_stream(s->cs);

        if (mode == DOUBLE_QUOTE && n == '"')
        {
            __pop_stream(s->cs);
            mode = NO_QUOTE;
        }

        else if (mode == SIMPLE_QUOTE && n == '\'')
        {
            __pop_stream(s->cs);
            mode = NO_QUOTE;
        }

        else if (mode == NO_QUOTE && n == '"')
        {
            __pop_stream(s->cs);
            mode = DOUBLE_QUOTE;
        }

        else if (mode == NO_QUOTE && n == '\'')
        {
            __pop_stream(s->cs);
            mode = SIMPLE_QUOTE;
        }

        else if (mode == NO_QUOTE && __interrupting_char(n))
        {
            break;
        }

        else
        {
            vec_push(&s->last_token_str, __pop_stream(s->cs));
        }
    }
}

int next_token(struct state *s)
{
    vec_reset(&s->last_token_str);

    // Eat whitespaces
    while (__peek_stream(s->cs) == ' ' || __peek_stream(s->cs) == '\t')
    {
        __pop_stream(s->cs);
    }

    // Check if reached end of stream
    if (__peek_stream(s->cs) == EOF)
        return T_EOF;

    __collect_next_token(s);

    return __longest_prefix_token(vec_cstring(&s->last_token_str));
}