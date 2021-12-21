#pragma once

#include <io/cstream.h>

#include "symtab.h"

/*!
 * \brief parser entry function
 * \param cs character stream
 * \param flag flags passed to parser & AST
 * \param exit_status Last command execution exit status
 * \param symtab The global symbols table
 * \return zero or a negative value in case of error
 */
int parser(struct cstream *cs, int flag, int *exit_status,
           struct symtab *symtab);
