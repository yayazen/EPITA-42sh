#include <stdio.h>

#include "rule.h"

int rl_expect(struct rl_state *s, int token)
{
    int rc = rl_accept(s, token);
    if (rc <= 0)
        fprintf(stderr, "rl_expect: unexpected token or error\n");
    return rc;
}
