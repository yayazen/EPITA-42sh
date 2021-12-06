#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#include "token.h"

#define LEX_LINE_START 1

/*!
 * \brief token structure set by the lexer
 */
struct token
{
    /* \see token.h */
    int key;
    /* must be freed */
    char *str;
};

/*!
 * \brief lexer entry function
 * \param a file stream
 * \param a pointer to a token struct where the result is stored
 * \param flags for the lexer
 * \return an error code defined in utils/error.h
 * \see token.h
 */
int cs_lex(struct cstream *cs, struct token *token, int flags);
