#include "parser.h"

#include "lexer.h"
#include "token.h"

/* For debugging purposes */
static inline void __debug(int token, struct vec *word)
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

    printf("%s ", type);
    if (token == T_LF)
        printf("['\\n']\n");
    else if (token == T_WORD)
        printf("['%s']\n", vec_cstring(word));
    else
        printf("['%s']\n", TOKEN_STR(token));
}

int cs_parse(struct cstream *cs, int flag)
{
    int rc = NO_ERROR;
    int lex_flag = LEX_LINE_START;
    int token;
    struct vec word;
    vec_init(&word);

    while (token != T_EOF)
    {
        token = cs_lex(cs, &word, lex_flag);

        if ((flag & 2) && token >= 0)
            __debug(token, &word);
        vec_reset(&word);

        if (token < 0)
        {
            if ((rc = -token) == KEYBOARD_INTERRUPT)
                continue;
            break;
        }

        lex_flag = (token == T_LF) ? LEX_LINE_START : 0;
    }
    vec_destroy(&word);
    return rc;
}
