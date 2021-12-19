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
     * \n -e option
     */
    INTERPRET_SPACERS = 1 << 1
};

/**
 * \brief Parse -n and -e arguments
 *
 * I did not use getopt to achieve this, because
 * it could conflict with the main function of this program...
 */
static char **__parse_mode(char **seek, int *mode)
{
    while (*seek)
    {
        if (!strcmp(*seek, "-ne") || !strcmp(*seek, "-en"))
            *mode |= INHIBIT_NEWLINE | INTERPRET_SPACERS;
        else if (!strcmp(*seek, "-n"))
            *mode |= INHIBIT_NEWLINE;
        else if (!strcmp(*seek, "-e"))
            *mode |= INTERPRET_SPACERS;
        else
            break;

        seek++;
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
    }
}

int bi_echo(char **args, struct symtab *s)
{
    assert(args && s);

    int mode = 0;
    char **seek = __parse_mode(args + 1, &mode);

    int first = 1;

    while (*seek)
    {
        if (!first)
            printf(" ");

        if (mode & INTERPRET_SPACERS)
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