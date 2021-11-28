/**
 * The lexer of the project
 */

#pragma once

#include "parser.h"

// static inline const char *__eatwhitespace(const char *s); // '\n' ' ' '   '
// static inline int __is_prefix(const char *s, const char *t);
// static inline int __longest_prefix_token(const char *s, const char *t);

int next_token(struct state *s);
