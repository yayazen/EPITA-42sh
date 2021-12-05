#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#define LEXER_LINE_START 1

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
 * \param stream a file stream
 * \param flag
 * \return int
 * \see token.h
 */
int cs_lex(struct cstream *cs, struct token *token, int flags);
