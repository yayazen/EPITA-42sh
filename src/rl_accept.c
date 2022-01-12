#include "constants.h"
#include "lexer.h"
#include "rule.h"
#include "token.h"

void rl_buffer_token(struct rl_state *s)
{
    vec_reset(&s->buffered_word);
    vec_pushstr(&s->buffered_word, vec_cstring(&s->word));
    s->buffered_token = s->token;
}

int rl_accept(struct rl_state *s, int token)
{
    while (s->err == KEYBOARD_INTERRUPT || s->flag & LEX_COLLECT)
    {
        if (s->buffered_token >= 0)
        {
            s->token = s->buffered_token;
            vec_reset(&s->word);
            vec_pushstr(&s->word, vec_cstring(&s->buffered_word));
            s->buffered_token = -1;
        }
        else
        {
            if (s->token == T_LF || s->token == T_SEMICOL
                || s->token == T_AND_IF || s->token == T_OR_IF)
                s->flag |= LEX_CMDSTART;
            lexer(s);
        }
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
