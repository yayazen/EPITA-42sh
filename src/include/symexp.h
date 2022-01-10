#pragma once

#include "ctx.h"
#include "list.h"

/**
 * \brief Expand a word, expecting a single result
 * \param ctx The context to use
 * \param word The word to expand
 */
char *symexp_word_single_result(const struct ctx *ctx, const char *word);

/**
 * \brief Expand a word
 * \param ctx The context to use
 * \param word The word to expand
 */
void symexp_word(const struct ctx *ctx, const char *word, struct list *dest);