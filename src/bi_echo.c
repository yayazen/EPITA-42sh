/**
 * \file bi_echo.c
 * \brief `echo` builtin implementation
 *
 * This builtin is not compliant with the POSIX standard
 *
 * This builtin accept the following parameters:
 * * `-n` : inhibit new lines
 * * `-e` : interpret `/n`, `//` and `\t`
 */

#include <assert.h>

#include "builtins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

enum
{
    /**
     * \brief -n option
     */
    INHIBIT_NEWLINE = 1 << 0,

    /**
     * \brief -e option
     */
    INTERPRET_SPACERS = 1 << 1,

    /**
     * \brief -E option
     */
    INHIBIT_BACKSLASH_INTERPRETATION = 1 << 2,
};

/**
 * \brief Parse -n and -e arguments
 *
 * I did not use getopt to achieve this, because
 * it could conflict with the main function of this program...
 */
static char **__parse_mode(char **seek, int *mode)
{
    bool stop = false;
    while (*seek && !stop)
    {
        if (seek[0][0] != '-' || seek[0][1] == '\0')
            break;

        int add_modes = 0;

        char *pos = (*seek) + 1;

        while (*pos && !stop)
        {
            if (*pos == 'n')
                add_modes |= INHIBIT_NEWLINE;
            else if (*pos == 'e')
                add_modes |= INTERPRET_SPACERS;
            else if (*pos == 'E')
                add_modes |= INHIBIT_BACKSLASH_INTERPRETATION;
            else
                stop = true;

            pos++;
        }

        if (!stop)
        {
            *mode |= add_modes;
            seek++;
        }
    }

    return seek;
}

/**
 * \brief My own strcpy implementation, where
 * `src` and `dest` may overlap
 */
static void __my_strcpy(const char *src, char *dest)
{
    while (*src)
    {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
}

/**
 * \brief Interpret escape characters
 */
void __interpret_escapes(char *message)
{
    while (*message)
    {
        if (*message != '\\')
        {
            message++;
            continue;
        }

        char c = 0;

        switch (*(message + 1))
        {
        case 'n':
            c = '\n';
            break;
        case 't':
            c = '\t';
            break;
        case '\\':
            c = '\\';
            break;
        }

        if (!c)
        {
            message++;
            continue;
        }

        *message = c;

        __my_strcpy(message + 2, message + 1);
        message++;
    }
}

int bi_echo(const struct ctx *ctx, char **args)
{
    assert(args && ctx);

    int mode = 0;
    char **seek = __parse_mode(args + 1, &mode);

    int first = 1;

    while (*seek)
    {
        if (!first)
            printf(" ");

        if (mode & INTERPRET_SPACERS
            && !(mode & INHIBIT_BACKSLASH_INTERPRETATION))
        {
            char *s = strdup(*seek);
            __interpret_escapes(s);
            printf("%s", s);
            free(s);
        }
        else
        {
            printf("%s", *seek);
        }

        first = 0;
        seek++;
    }

    if (!(mode & INHIBIT_NEWLINE))
        printf("\n");

    fflush(stdout);

    return 0;
}