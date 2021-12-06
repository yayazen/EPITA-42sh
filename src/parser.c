#include "parser.h"

#include "lexer.h"
#include "token.h"

/* For debugging purposes */
static inline void __debug(struct token *t)
{
    if (t->key == EOF)
        return;

    const char *type;
    switch (TOKEN_TYPE(t->key))
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

    printf("%s ", type);

    if (t->key == T_LF)
        printf("['\\n']\n");
    else if (t->key == T_WORD)
        printf("['%s']\n", t->str);
    else
        printf("['%s']\n", TOKEN_STR(t->key));
}

int cs_parse(struct cstream *cs, int flag)
{
    int rc = 0;

    struct token token = { .key = 0, .str = NULL };
    while (1)
    {
        int rc = cs_lex(cs, &token, LEX_LINE_START);

        /* --debug */
        if (flag & 2)
            __debug(&token);

        free(token.str);

        if (rc == KEYBOARD_INTERRUPT)
            continue;
        if (token.key == EOF)
            break;
    }

    return rc;
}
