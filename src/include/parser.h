#pragma once

#include <io/cstream.h>

/*!
 * \brief parser entry function
 * \param a character stream
 * \return zero or a negative value in case of erroro
 */
int cs_parse(struct cstream *cs, int flag);
