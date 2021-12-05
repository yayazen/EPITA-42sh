#pragma once

#include <stddef.h>

/*!
 * \file token.h
 * \brief Token definitions header file.
 *
 *
 * \file token.def
 * \brief Token definitions X-Macro file
 * \include{lineno} token.def
 * \var   TOKEN(Key, Str, Type)
 * \param Key The enumeration value of a token.
 * \param Str The string representation of a token.
 * \param Type The type of a token
 *      * DEFAULT :  subject to expansion and globbing
 *          - WORD | IONUMBER | ASSIGN | HEREDOC | ...
 *      * SPECIAL : special meaning token
 *          - hold only if not in ("" | '') else WORD
 *      * KEYWORD : reserved token
 *          - hold only if KEYWORD is the first token of a command else WORD
 *      * BUILTIN : shell builtins
 *          - hold only if BUILTIN is the first token of a command else WORD
 */

/*!
 * See _token_get_type().
 */
#define TOKEN_TYPE(Key) _token_get_type(Key)

/*!
 * See _token_get_str().
 */
#define TOKEN_STR(Key) _token_get_str(Key)

/*!
 * See _token_get_strlen().
 */
#define TOKEN_STRLEN(Key) _token_get_strlen(Key)

/*!
 * Upper bound for TOKEN_STRLEN
 */
#define TOKEN_UBLEN 32

/// \cond DEV_DOC
enum
{
#define TOKEN(Key, Str, Type) Key,
#include "token.def"
#undef TOKEN
    TOKEN_COUNT
};
/// \endcond

enum
{
    DEFAULT,
    SPECIAL,
    KEYWORD,
    BUILTIN
};

/*!
 * \brief Get the type of a token.
 * \param key     A token key as described in token.def
 * \return The associated type as defined in token.def
 * \warning should not be used! use TOKEN_TYPE(KEY) macro instead.
 */
int _token_get_type(register int key);

/*!
 * \brief Get the string of a token.
 * \param key     A token key as described in token.def
 * \return The associated string as defined in token.def
 * \warning should not be used! use TOKEN_STR(KEY) macro instead.
 */
const char *_token_get_str(register int key);

/*!
 * \brief Get the string length of a token.
 * \param key      A token key as described in token.def
 * \return The length of the associated string as defined in token.def
 * \warning should not be used! use TOKEN_STRLEN(KEY) macro instead.
 */
size_t _token_get_strlen(register int key);
