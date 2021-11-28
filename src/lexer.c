#include "ast/lexer.h"

#include "ast/token.h"

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

int next_token(struct state *s)
{
    // static const char *next = NULL;

    s->s = __eat_whitespaces(s->s);

    int token = __longest_prefix_token(s);

    if (token == T_WORD)
    {
        *s->saveptr = s;
        // TODO
    }

    return token;
}