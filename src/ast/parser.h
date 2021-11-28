/**
 * The parser
 */

#pragma once

struct state
{
    int token;
    int flags;
    const char *s;
    const char **saveptr;
};

int accept(int token);

int rule_input(const char *s);

int parse(int flags, const char *s);