#include <stdlib.h>
#include <string.h>
#include <utils/error.h>
#include <utils/vec.h>

#include "constants.h"
#include "lexer.h"
#include "rule.h"
#include "token.h"

int rl_accept(struct rl_state *s, int token, int rltype)
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
        if (rltype != RL_NORULE)
        {
            s->ast = rl_ast_new(rltype);
            if (!s->ast || !(s->ast->word = strdup(vec_cstring(&s->word))))
            {
                rl_ast_free(s->ast);
                s->ast = NULL;
                return -(s->err = UNKNOWN_ERROR);
            }
            s->ast->type = rltype;
        }
        s->flag |= LEX_COLLECT;
        return true;
    }
    return false;
}
