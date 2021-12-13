#include "quotes.h"

#include "stdio.h"

enum __simple_quote_status
{
    DEFAULT,
    IN_SIMPLE_QUOTE,
    IN_DOUBLE_QUOTE
};

/**
 * \brief My own strcpy implementation, where
 * `src` and `dest` may overlap
 */
void __my_strcpy(char *src, char *dest)
{
    while (*src)
    {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
}

char *expand_simple_quotes(char *str)
{
    char *pos = str;
    enum __simple_quote_status status = DEFAULT;

    while (*pos)
    {
        if (status != IN_DOUBLE_QUOTE && *pos == '\'')
        {
            status = status == DEFAULT ? IN_SIMPLE_QUOTE : DEFAULT;
            __my_strcpy(pos + 1, pos);
            continue;
        }

        if (status != IN_SIMPLE_QUOTE && *pos == '"')
        {
            status = status == DEFAULT ? IN_DOUBLE_QUOTE : IN_SIMPLE_QUOTE;
        }

        pos++;
    }

    return str;
}
