#include <stdlib.h>
#include <string.h>
#include <utils/error.h>
#include <utils/vec.h>

#include "constants.h"
#include "lexer.h"
#include "rule.h"

int rl_accept(struct rl_state *s, int token, int rl_type)
{
    while (s->err == KEYBOARD_INTERRUPT || s->flag & LAST_TOKEN_EATEN)
    {
        lexer(s);
        s->flag &= (~LAST_TOKEN_EATEN);
    }

    if (s->err)
        return -s->err;

    if (s->token == token)
    {
        if (rl_type != RL_NORULE)
        {
            s->ast = calloc(1, sizeof(struct rl_ast));
            if (!s->ast || !(s->ast->word = strdup(vec_cstring(&s->word))))
            {
                rl_ast_free(s->ast);
                return UNKNOWN_ERROR;
            }
            s->ast->type = rl_type;
        }

        s->flag |= LAST_TOKEN_EATEN;
        return 1;
    }
    return 0;
}
