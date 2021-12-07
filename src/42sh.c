#include <errno.h>
#include <getopt.h>
#include <io/cstream.h>
#include <stdio.h>
#include <unistd.h>

#include "parser.h"

#define PACKAGE "42sh"
#define VERSION "0.1.0"

enum
{
    /* pow_2 */
    OPT_HELP = 1,
    OPT_DEBUG = 2,
};

static const char *__usages =
    PACKAGE ", version " VERSION "\n"
            "Usage: " PACKAGE " [OPTIONS] [FILE] [ARGUMENTS...]\n"
            "Long options:\n"
            "       --help          shows the help menu\n"
            "       --debug         enable debug output\n"
            "Shell options:\n"
            "       -c command      evaluates argument as command";

static int optflag;
static const struct option long_opts[] = {
    { "help", no_argument, &optflag, OPT_HELP },
    { "debug", no_argument, &optflag, OPT_DEBUG },
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
        case '?':
        default:
            return 1;
        }
    }

    *cs = (*cs)           ? *cs
        : (optind < optc) ? cstream_file_create(fopen(optv[optind], "r"), true)
        : (isatty(STDIN_FILENO)) ? cstream_readline_create()
                                 : cstream_file_create(stdin, false);

    return *cs == NULL;
}

int main(int argc, char *argv[])
{
    int flag = 0;
    struct cstream *cs = NULL;
    int err = __parse_opts(argc, argv, &cs, &flag);
    if (err || (flag & OPT_HELP))
        goto err_parse_args;

    err = cs_parse(cs, flag);

    goto end_loop;
err_parse_args:
    puts(__usages);
end_loop:
    perror(PACKAGE);
    if (cs)
        cstream_free(cs);
    return err;
}
