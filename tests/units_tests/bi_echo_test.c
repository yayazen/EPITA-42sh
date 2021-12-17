#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <string.h>

#include "builtins.h"

TestSuite(bi_echo, .init = cr_redirect_stdout, .timeout = 15);

#define BUFF_SIZE 1000

static void __check_stdout_content(const char *expected)
{
    char stdout[BUFF_SIZE] = { 0 };
    FILE *f = cr_get_redirected_stdout();
    fread(stdout, BUFF_SIZE - 1, 1, f);
    fclose(f);

    cr_assert_str_eq(stdout, expected);
}

Test(bi_echo, simple)
{
    char *args[] = { "echo", "a", "b", "c", NULL };
    bi_echo(args);
    cr_assert_stdout_eq_str("a b c\n");
}

Test(bi_echo, single)
{
    char *args[] = { "echo", "a", NULL };
    bi_echo(args);
    cr_assert_stdout_eq_str("a\n");
}

Test(bi_echo, single_no_newline)
{
    char *args[] = { "echo", "-n", "a", NULL };
    bi_echo(args);
    cr_assert_stdout_eq_str("a");
}

Test(bi_echo, empty)
{
    char *args[] = { "echo", NULL };
    bi_echo(args);
    cr_assert_stdout_eq_str("\n");
}

Test(bi_echo, only_args)
{
    char *args[] = { "echo", "-ne", NULL };
    bi_echo(args);
    __check_stdout_content("");
}

Test(bi_echo, expansion_1)
{
    char *args[] = { "echo", "-e", "t\\nt", NULL };
    bi_echo(args);
    cr_assert_stdout_eq_str("t\nt\n");
}

Test(bi_echo, expansion_2)
{
    char *args[] = { "echo", "-e", "t\\n\\\\ t", NULL };
    bi_echo(args);
    __check_stdout_content("t\n\\ t\n");
}