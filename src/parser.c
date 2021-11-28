#include "ast/parser.h"

#include "ast/lexer.h"

static struct state s;

int accept(int token)
{
    if (s.token == token)
    {
        s.token = next_token(&s);
        return 1;
    }

    return 0;
}