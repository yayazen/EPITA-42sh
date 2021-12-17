#include <assert.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"

static void __invoke_cd(char *arg)
{
    char *args[] = { "cd", arg, NULL };
    cr_assert_eq(bi_cd(args), 0);
}

TestSuite(bi_cd, .init = cr_redirect_stdout, .timeout = 15);

Test(bi_cd, check_env_vars)
{
    __invoke_cd("/tmp");
    __invoke_cd("/bin");
    cr_assert_str_eq("/tmp", getenv("OLDPWD"));
    cr_assert_str_eq("/bin", getenv("PWD"));
}

Test(bi_cd, check_minus)
{
    __invoke_cd("/tmp");
    __invoke_cd("/bin");
    cr_assert_str_eq("/tmp", getenv("OLDPWD"));
    cr_assert_str_eq("/bin", getenv("PWD"));

    __invoke_cd("-");
    cr_assert_str_eq("/bin", getenv("OLDPWD"));
    cr_assert_str_eq("/tmp", getenv("PWD"));

    __invoke_cd("-");
    cr_assert_str_eq("/tmp", getenv("OLDPWD"));
    cr_assert_str_eq("/bin", getenv("PWD"));
}

Test(bi_cd, check_invalid)
{
    cr_redirect_stderr();
    char *args[] = { "cd", "/nonexisting", NULL };
    cr_assert_neq(bi_cd(args), 0);
    cr_assert_stdout_neq_str("");
}

Test(bi_cd, validate_expansion)
{
    __invoke_cd("/proc");
    cr_assert_str_eq("/proc", getenv("PWD"));

    __invoke_cd("sys");
    cr_assert_str_eq("/proc/sys", getenv("PWD"));

    __invoke_cd("..");
    cr_assert_str_eq("/proc", getenv("PWD"));

    __invoke_cd("bin/../../tmp");
    cr_assert_str_eq("/tmp", getenv("PWD"));

    __invoke_cd("/bin/../../tmp");
    cr_assert_str_eq("/tmp", getenv("PWD"));
}

Test(bi_cd, no_argument_with_home)
{
    __invoke_cd("/proc");
    cr_assert_str_eq("/proc", getenv("PWD"));

    assert(setenv("HOME", "/bin", 1) == 0);
    char *args[] = { "cd", NULL };
    cr_assert_eq(bi_cd(args), 0);
    cr_assert_str_eq("/bin", getenv("PWD"));
}

Test(bi_cd, no_argument_without_home)
{
    __invoke_cd("/proc");
    cr_assert_str_eq("/proc", getenv("PWD"));

    assert(unsetenv("HOME") == 0);
    char *args[] = { "cd", NULL };
    cr_assert_eq(bi_cd(args), 0);
    cr_assert_str_eq("/proc", getenv("PWD"));
}

Test(bi_cd, no_argument_with_empty_home)
{
    __invoke_cd("/proc");
    cr_assert_str_eq("/proc", getenv("PWD"));

    assert(setenv("HOME", "", 1) == 0);
    char *args[] = { "cd", NULL };
    cr_assert_eq(bi_cd(args), 0);
    cr_assert_str_eq("/proc", getenv("PWD"));
}