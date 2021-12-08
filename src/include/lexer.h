#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#define LEX_LINE_START 1

/*!
 * \brief lexer entry function
 * \param a file stream
 * \param a vector to store the matching substring
 * \param flags for the lexer
 * \return a token (a positive value) or an negative error value
 *          as defined in utils/error.h
 * \see token.h
 */
int cs_lex(struct cstream *cs, struct vec *word, int flag);
