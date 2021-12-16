#include <stdio.h>

#include "rule.h"
#include "token.h"

static const char *__token_str(int token)
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

int rl_expect(struct rl_state *s, int token)
{
    int rc = rl_accept(s, token);
    if (rc <= 0)
        fprintf(stderr,
                "42sh: syntax error near unexpected token <%s> expected <%s>\n",
                __token_str(s->token), __token_str(token));
    return rc;
}
