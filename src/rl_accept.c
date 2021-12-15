#include <stdlib.h>
#include <string.h>
#include <utils/error.h>
#include <utils/vec.h>

#include "constants.h"
#include "lexer.h"
#include "rule.h"
#include "token.h"

//static inline int __exectree(struct rl_state *s, int rltype)
//{
//    if (rltype == RL_NORULE)
//        return NO_ERROR;
//
//    if (!s->node && !(s->node = rl_exectree_new(rltype)))
//        return -(s->err = UNKNOWN_ERROR);
//
//    if (rltype != RL_SIMPLE_CMD)
//        return NO_ERROR; 
//}
//

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
            s->node = rl_exectree_new(rltype);
            if (!s->node || !(s->node->word = strdup(vec_cstring(&s->word))))
            {
                rl_exectree_free(s->node);
                s->node = NULL;
                return -(s->err = UNKNOWN_ERROR);
            }
            s->node->type = rltype;
        }
        s->flag |= LEX_COLLECT;
        return true;
    }
    return false;
}
