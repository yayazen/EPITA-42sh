/**
 * \file builtins.c
 * \brief General builtin implementation functions
 */

#include "builtins.h"

#include "string.h"

builtin_def builtin_find(const char *name)
{
#define BUILTIN(n, func)                                                       \
    if (strcmp(name, n) == 0)                                                  \
        return func;
#include "builtins.def"
#undef BUILTIN

    return NULL;
}