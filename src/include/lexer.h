#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#define LEX_LINE_START 1

/*!
 * \brief lexer entry function
 * \param a file stream
 * \param a pointer to a token struct where the result is stored
 * \param flags for the lexer
 * \return an error code defined in utils/error.h
 * \see token.h
 */
int cs_lex(struct cstream *cs, struct vec *word, int flags);
