#include <stdio.h>

#include "rule.h"
#include "token.h"

static const char *__pretty_token(int token)
{
    switch (token)
    {
    case T_LF:
        return "newline";
    case T_WORD:
        return "word";
    case T_EOF:
        return "EOF";
    }
    return TOKEN_STR(token);
}

int rl_expect(struct rl_state *s, int token, int rl_type)
{
    int rc = rl_accept(s, token, rl_type);
    if (rc <= 0)
        fprintf(stderr,
                "42sh: syntax error near unexpected token <%s> expected <%s>\n",
                __pretty_token(s->token), __pretty_token(token));
    return rc;
}
