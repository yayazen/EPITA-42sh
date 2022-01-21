#pragma once

/** \brief left trim string. Returns a pointer on the first non-space or tabs
 * character or on '\0' if the string is blank */
static inline const char *ltrim(const char *input)
{
    while (*input == '\t' || *input == ' ')
        input++;
    return input;
}
