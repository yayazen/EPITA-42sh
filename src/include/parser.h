#pragma once

#include <io/cstream.h>

/*!
 * \brief parser entry function
 * \param stream a character stream
 * \return int
 */
int cs_parse(struct cstream *cs, int flag);
