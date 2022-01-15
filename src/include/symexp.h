#pragma once

#include <utils/vec.h>

#include "ctx.h"
#include "list.h"

struct ctx;
struct list;

/** \brief expansion state */
struct symexp_state
{
    /** current execution context */
    const struct ctx *ctx;
    /** currently expanded word */
    const char *word;
    /** destination list */
    struct list *dest;
    /** expansion vector */
    struct vec expvec;
    /** lexer mode */
    int mode;
    /** currently parsed variable length */
    int i;
    /* currently parsed variable key */
    char key[100];
    /** some special variables needs to write somewhere temporary */
    char exp_buff[10];
    /** currently processed character */
    char c;
};

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