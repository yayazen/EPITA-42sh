#include <errno.h>
#include <getopt.h>
#include <io/cstream.h>
#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "parser.h"
#include "symtab.h"

static const char *__usages =
    PACKAGE ", version " VERSION "\n"
            "Usage: " PACKAGE " [OPTIONS] [FILE] [ARGUMENTS...]\n"
            "Long options:\n"
            "       --help          shows the help menu\n"
            "       --debug         enable debug output\n"
            "       --print-ast     print the ast after parsing\n"
            "       --print-ast-dot print the ast in dot format after parsing\n"
            "Shell options:\n"
            "       -c command      evaluates argument as command";

static int optflag;
static const struct option long_opts[] = {
    { "help", no_argument, &optflag, OPT_HELP },
    { "debug", no_argument, &optflag, OPT_DEBUG },
    { "print-ast", no_argument, &optflag, OPT_PRINT_AST },
    { "print-ast-dot", no_argument, &optflag, OPT_PRINT_AST_DOT },
    { NULL, 0, NULL, 0 }
};

static int __parse_opts(int optc, char **optv, struct cstream **cs, int *flag)
{
    *cs = NULL;
    while (1)
    {
        char val = getopt_long(optc, optv, "c:", long_opts, NULL);
        if (val == -1)
            break;

        switch (val)
        {
        case 0:
            *flag |= optflag;
            break;
        case 'c':
            if (optarg && !(*cs))
            {
                *cs = cstream_string_create(optarg);
                break;
            }
            __attribute__((fallthrough));
        default:
            return 1;
        }
    }

    if (*cs)
        ;
    else if (optind < optc)
        *cs = cstream_file_create(fopen(optv[optind], "r"), true);
    else if (isatty(STDIN_FILENO))
        *cs = cstream_readline_create();
    else
        *cs = cstream_file_create(stdin, false);

    return *cs == NULL;
}

int main(int argc, char *argv[], char **envp)
{
    int flag = 0;
    struct cstream *cs = NULL;
    int exit_status = 0;
    struct symtab *symtab = NULL;
    int err = __parse_opts(argc, argv, &cs, &flag);

    if (flag & OPT_HELP)
        goto err_parse_args;
    if (err)
        goto end_loop;

    symtab = symtab_new();
    symtab_fill_with_env_vars(symtab, envp);

    while ((err = parser(cs, flag, &exit_status, symtab)) == NO_ERROR
           && !(flag & OPT_HELP))
    {
        ;
    }

    goto end_loop;
err_parse_args:
    puts(__usages);
end_loop:
    if (err && err != REACHED_EOF)
        perror(PACKAGE);
    if (flag & OPT_PRINT_AST)
        symtab_print(symtab);
    symtab_free(symtab);
    if (cs)
        cstream_free(cs);
    if (err == REACHED_EOF)
        return exit_status;

    // To pass moulette tests, exit status must be 2 in case of parsing error
    return err == PARSER_ERROR ? 2 : err;
}
