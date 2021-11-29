/**
 * The parser
 */

#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

struct state
{
    int token;
    int flags;
    struct cstream *cs;
    struct vec last_token_str;
};

int accept(int token);

int rule_input(const char *s);

int parse(int flags, const char *s);