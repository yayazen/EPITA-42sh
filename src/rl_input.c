#include "rule.h"
#include "stdio.h"
#include "token.h"

static void __debug(struct rl_state *s)
{
    if (!(s->flag & 2))
        return;

    switch (TOKEN_TYPE(s->token))
    {
    case SPECIAL:
        printf("SPECIAL");
        break;
    case KEYWORD:
        printf("KEYWORD");
        break;
    default:
        printf("DEFAULT");
        break;
    }

#define TOKEN(Key, Str, Type)                                                  \
    if (Key == s->token)                                                       \
        printf(" | " #Key);
#include "token.def"
#undef TOKEN

    if (s->token == T_LF)
        printf(" | ['\\n']\n");
    else
        printf(" | ['%s']\n", vec_cstring(&s->word));
}

/* Match anything you give it for debug purposes */
int rl_input(struct rl_state *s)
{
    int rc;

    s->flag |= 1;
    while ((rc = rl_accept(s, T_EOF)) != 1)
    {
        if (rc < 0)
        {
            perror("rl_input");
            break;
        }

        __debug(s);

        if (s->token == T_LF || s->token == T_SEMICOL)
            s->flag |= 1;
        else
            s->flag &= ~1;

        rl_accept(s, s->token);
    }
    return rc;
}
