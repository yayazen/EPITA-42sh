#include <stdlib.h>
#include <string.h>
#include <utils/error.h>
#include <utils/vec.h>

#include "lexer.h"
#include "rule.h"

int rl_accept(struct rl_state *s, int token, int rl_type)
{
    while (s->token == -KEYBOARD_INTERRUPT)
        s->token = cs_lex(s->cs, &s->word, s->flag);

    if (s->token < 0)
        return s->token;

    if (s->token == token)
    {
        if (rl_type != RL_VOID)
        {
            s->ast = calloc(1, sizeof(struct rl_ast));
            if (!s->ast || !(s->ast->word = strdup(vec_cstring(&s->word))))
            {
                rl_ast_free(s->ast);
                return -1;
            }
            s->ast->type = rl_type;
        }
        s->token = cs_lex(s->cs, &s->word, s->flag);
        return 1;
    }
    return 0;
}
