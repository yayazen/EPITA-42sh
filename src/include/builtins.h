#pragma once

#include "symtab.h"

/**
 * \brief Builtin prototype
 */
typedef int (*builtin_def)(char **args, struct symtab *symtab);

/**
 * Attempt to find a builtin with a given name
 *
 * \param name The name of the builtin to get
 * \return A pointer on the builtin function, or NULL
 * if no builtin was found
 */
builtin_def builtin_find(const char *name);

/**
 * Declare builtins
 */
#define BUILTIN(name, func) int func(char **args, struct symtab *symtab);
#include "builtins.def"
#undef BUILTIN
