#include "constants.h"
#include "lexer.h"
#include "rule.h"
#include "token.h"

int rl_accept(struct rl_state *s, int token)
{
    while (s->err == KEYBOARD_INTERRUPT || s->flag & LEX_COLLECT)
    {
        if (s->token == T_LF || s->token == T_SEMICOL)
            s->flag |= LEX_CMDSTART;
        lexer(s);
        s->flag &= ~(LEX_COLLECT | LEX_CMDSTART);
    }

    if (s->err != NO_ERROR)
        return -s->err;

    if (s->token == token)
    {
        s->flag |= LEX_COLLECT;
        return true;
    }
    return false;
}
