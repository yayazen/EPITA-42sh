#include "lexer.h"
#include "rule.h"
#include "token.h"

int rl_accept(struct rl_state *s, int token)
{
    while (s->token == -KEYBOARD_INTERRUPT)
        s->token = cs_lex(s->cs, &s->word, s->flag);

    if (s->token < 0)
        return s->token;

    if (s->token == token)
    {
        s->token = cs_lex(s->cs, &s->word, s->flag);
        return 1;
    }
    return 0;
}
