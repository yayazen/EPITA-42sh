#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"

/**
 * \brief Set new active directory to `dir`.
 * In case of error, a message on stderr is shown
 * \param dir New directory name
 * \return 0 in case of success, or a positive value otherwise.
 */
int __chdir(char *dir)
{
    if (dir == NULL)
    {
        perror("cd : dir was NULL");
        return 1;
    }

    char new_old_pwd[PATH_MAX + 1];
    assert(getcwd(new_old_pwd, sizeof(new_old_pwd)));

    int res = chdir(dir);

    if (res)
        perror("cd");

    else
    {
        assert(!setenv("PWD", dir, 1));
        assert(!setenv("OLDPWD", new_old_pwd, 1));
    }

    return !res;
}

/**
 * cd builtin
 *
 * \ref https://pubs.opengroup.org/onlinepubs/9699919799/utilities/cd.html
 */
int bi_cd(char **args)
{
    // No argument
    if (!args[1])
    {
        // If HOME variable is defined
        char *home = getenv("HOME");
        if (*home)
        {
            return __chdir(home);
        }

        return 0;
    }

    /* cd - */
    if (!strcmp(args[1], "-"))
    {
        char *oldpwd = getenv("OLDPWD");
        printf("%s\n", oldpwd);
        return __chdir(oldpwd);
    }

    return __chdir(args[1]);
}