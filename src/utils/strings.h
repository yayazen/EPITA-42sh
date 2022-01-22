#pragma once

/** \brief Check if a given character is a digit or not */
#define is_digit(c) (c >= '0' && c <= '9')

/** \brief Check if a given string contains only an integer or not*/
static bool is_int(const char *val)
{
    if (!val || !*val)
        return false;
    while (*val && is_digit(*val))
        val++;
    return *val == '\0';
}

/** \brief left trim string. Returns a pointer on the first non-space or tabs
 * character or on '\0' if the string is blank */
static const char *ltrim(const char *input)
{
    while (*input == '\t' || *input == ' ')
        input++;
    return input;
}
