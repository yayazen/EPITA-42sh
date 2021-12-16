#pragma once

#include <io/cstream.h>

/*!
 * \brief parser entry function
 * \param cs character stream
 * \param flag flags passed to parser & AST
 * \param exit_status Last command execution exit status
 * \return zero or a negative value in case of erroro
 */
int parser(struct cstream *cs, int flag, int *exit_status);
