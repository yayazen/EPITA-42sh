#pragma once

#include "symtab.h"

/**
 * \brief Expand a word
 * \param symtab Symbols table to use
 * \param word The word to expand
 */
char *symexp_word(struct symtab *symtab, const char *word);