#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "ctx.h"

#define JOIN_BUFF_LEN PATH_MAX + PATH_MAX + 4

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
 * \brief Determine current working directory location
 * Attempt to do it from  PWD environment variable
 * If this variable is missing, use getcwd as a fallback
 * \param dest A buffer large enough to contain any kind of path
 */
static void __get_working_directory(char *dest)
{
    char *cur_pwd = getenv("PWD");
    if (cur_pwd)
        strcpy(dest, cur_pwd);
    else
        assert(getcwd(dest, PATH_MAX + 1));
}

/**
 * \brief Join two directories
 * \param dest Destination buffer, large enough to contain
 * both `a` and `b`
 * \param a The first directory (the "parent" directory)
 * \param b The second directory (the "child" directory)
 * If `b` starts with a '/' then the content of `a` is ignored
 */
static void __join_dirs(char *dest, const char *a, const char *b)
{
    assert(dest && a && b && *a && *b);

    if (b[0] == '/')
    {
        snprintf(dest, JOIN_BUFF_LEN - 3, "%s/", b);
    }
    else
    {
        snprintf(dest, JOIN_BUFF_LEN - 3, "%s/%s/", a, b);
    }

    char *seek = dest;

    // Process the string
    while (*seek)
    {
        // Skip non-slash characters
        if (*seek != '/')
            seek++;

        // Remove double slash
        else if (*(seek + 1) == '/')
            __my_strcpy(seek + 1, seek);

        // Remove '/./'
        else if (*(seek + 1) == '.' && *(seek + 2) == '/')
            __my_strcpy(seek + 2, seek);

        // Process '/../'
        else if (*(seek + 1) == '.' && *(seek + 2) == '.' && *(seek + 3) == '/')
        {
            char *cpy_dest = dest < seek ? seek - 1 : dest;
            while (*cpy_dest != '/')
                cpy_dest--;
            __my_strcpy(seek + 3, cpy_dest);
            seek = cpy_dest;
        }

        else
            seek++;
    }

    // Remove last slash
    if (*(dest + 1))
        *(seek - 1) = '\0';
}

/**
 * \brief Set new active directory to `dir`.
 * In case of error, a message on stderr is shown
 * \param dir New directory name
 * \return 0 in case of success, or a positive value otherwise.
 */
static int __chdir(char *dir, struct symtab *s)
{
    if (dir == NULL)
    {
        perror("cd : dir was NULL");
        return 1;
    }

    char new_old_pwd[JOIN_BUFF_LEN];
    __get_working_directory(new_old_pwd);

    char new_pwd[JOIN_BUFF_LEN];
    __join_dirs(new_pwd, new_old_pwd, dir);

    int res = chdir(new_pwd);

    if (res)
    {
        fprintf(stderr, "cd to dir %s - ", new_pwd);
        perror("cd");
    }

    else
    {
        assert(!setenv("PWD", new_pwd, 1));
        assert(!setenv("OLDPWD", new_old_pwd, 1));

        symtab_add(s, "PWD", KV_WORD, strdup(new_pwd))->value.word.exported = 1;
        symtab_add(s, "OLDPWD", KV_WORD, strdup(new_old_pwd))
            ->value.word.exported = 1;
    }

    return res == 0 ? 0 : 1;
}

/**
 * cd builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/9699919799/utilities/cd.html
 */
int bi_cd(const struct ctx *ctx, char **args)
{
    assert(args && ctx);

    // No argument
    if (!args[1])
    {
        // If HOME variable is defined
        char *home = getenv("HOME");
        if (home && *home)
        {
            return __chdir(home, ctx->st);
        }

        return 0;
    }

    /* cd - */
    if (!strcmp(args[1], "-"))
    {
        char *oldpwd = getenv("OLDPWD");
        printf("%s\n", oldpwd);
        fflush(stdout);
        return __chdir(oldpwd, ctx->st);
    }

    return __chdir(args[1], ctx->st);
}