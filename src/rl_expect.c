#include <stdio.h>

#include "rule.h"
#include "token.h"

int rl_expect(struct rl_state *s, int token, int rl_type)
{
    int rc = rl_accept(s, token, rl_type);
    if (rc <= 0)
        fprintf(stderr, "42sh: syntax error near unexpected token <%s>\n",
                (s->token == T_LF) ? "newline" : TOKEN_STR(s->token));
    return rc;
}
