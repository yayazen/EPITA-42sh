#include "rule.h"
#include "stdio.h"
#include "token.h"

static const char *__type_str(int token)
{
    const char *type;
    switch (TOKEN_TYPE(token))
    {
    case SPECIAL:
        type = "SPECIAL";
        break;
    case KEYWORD:
        type = "KEYWORD";
        break;
    default:
        type = "DEFAULT";
        break;
    }
    return type;
}

/* Match anything you give it for debug purposes */
int rl_input(struct rl_state *s)
{
    int rc;

    s->flag = 1;
    while ((rc = rl_accept(s, T_EOF)) != 1)
    {
        if (rc < 0)
        {
            break;
            perror("rl_input");
        }

        printf("%s ", __type_str(s->token));
        if (s->token == T_LF)
            printf("['\\n']\n");
        else
            printf("['%s']\n", vec_cstring(&s->word));

        s->flag = (s->token == T_LF || s->token == T_SEMICOL);
        rl_accept(s, s->token);
    }
    return rc;
}
