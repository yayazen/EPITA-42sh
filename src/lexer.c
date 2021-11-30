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

/**
 * \brief Read the next character on the stream, consuming it
 * \return A character
 */
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

/**
 * \brief Read the next character on the stream without consuming it
 * \return A character
 */
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

/**
 * \brief Check whether a character indicates if we reached a new token
 * \return 1 if such token was reached
 */
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

/**
 * \brief Read the next character on the stream, saving
 *        it in the `last_token_str` variable
 */
static void __collect_next_token(struct state *s)
{
    enum READMODE mode = NO_QUOTE;

    while (__peek_stream(s->cs) != EOF)
    {
        char n = __peek_stream(s->cs);

        // Switch from double quote mode to no quote
        if (mode == DOUBLE_QUOTE && n == '"')
        {
            __pop_stream(s->cs);
            mode = NO_QUOTE;
        }

        // Switch from simple quote mode to no quote
        else if (mode == SIMPLE_QUOTE && n == '\'')
        {
            __pop_stream(s->cs);
            mode = NO_QUOTE;
        }

        // Switch from no quote mode to double quote
        else if (mode == NO_QUOTE && n == '"')
        {
            __pop_stream(s->cs);
            mode = DOUBLE_QUOTE;
        }

        // Switch from no quote mode to simple quote
        else if (mode == NO_QUOTE && n == '\'')
        {
            __pop_stream(s->cs);
            mode = SIMPLE_QUOTE;
        }

        // Check if we reached an interrupting character (<>\n...)
        else if (mode == NO_QUOTE && __interrupting_char(n))
        {
            if (s->last_token_str.size == 0)
                vec_push(&s->last_token_str, __pop_stream(s->cs));
            break;
        }

        // Save the character
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