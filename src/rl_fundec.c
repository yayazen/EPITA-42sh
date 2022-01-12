#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_fundec(struct rl_state *s)
{
    // This is the only part of the grammar which is LL(1)
    if (rl_accept(s, T_WORD) <= 0)
        return -s->err;

    s->flag &= ~LEX_CMDSTART;
    char *funcname = strdup(vec_cstring(&s->word));

    // We need to push back on the stream the word
    if (rl_accept(s, T_LPAR) <= 0)
    {
        // Swap current token in state and word
        rl_buffer_token(s);
        vec_reset(&s->word);
        vec_pushstr(&s->word, funcname);
        s->token = T_WORD;

        free(funcname);
        return false;
    }

    // TODO : write function definitions
    printf("go on with function %s definition !\n", funcname);
    exit(0);
    return (s->err != NO_ERROR) ? -s->err : true;
}