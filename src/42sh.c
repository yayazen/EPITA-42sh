#include <ast/lexer.h>
#include <ast/token.h>
#include <err.h>
#include <getopt.h>
#include <io/cstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/vec.h>

/**
 * \brief Run getopt_long on command line arguments and take appropriate actions
 * \return Nothing
 */
static void run_getopt(int argc, char **argv, struct cstream **stream, int *err)
{
    int option_index = 0;

    struct option options[] = { { "command", required_argument, 0, 'c' },
                                { 0, 0, 0, 0 } };

    while (!*err)
    {
        int c = getopt_long(argc, argv, "c:", options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            // TODO : handle long options (when there are some)
            break;

        case 'c':
            if (*stream == NULL)
                *stream = cstream_string_create(optarg);
            break;

        default:
            *err = 1;
        }
    }
}

/**
 * \brief Parse the command line arguments
 * \return A character stream
 */
static struct cstream *parse_args(int argc, char *argv[])
{
    // If launched without argument, read the standard input
    if (argc == 1)
    {
        if (isatty(STDIN_FILENO))
            return cstream_readline_create();
        return cstream_file_create(stdin, /* fclose_on_free */ false);
    }

    int err = 0;
    struct cstream *stream = NULL;

    // Parse arguments
    run_getopt(argc, argv, &stream, &err);

    // Open file stream only in case of success
    if (optind < argc && !err && stream == NULL)
    {
        // Handle non-option argv elements
        FILE *fp = fopen(argv[optind++], "r");
        if (fp == NULL)
        {
            warn("failed to open script file");
            return NULL;
        }

        stream = cstream_file_create(fp, /* fclose_on_free */ true);
    }

    if (optind < argc && !err)
    {
        // TODO : add support for optional arguments
        warn("Optional arguments not supported yet!");
    }

    if (err)
        fprintf(stderr, "Usage: %s [-c COMMAND] [SCRIPT] [ARGUMENTS ...]\n",
                argv[0]);

    if (err && stream != NULL)
    {
        cstream_free(stream);
        stream = NULL;
    }

    return stream;
}

/**
 * \brief Read and print lines on newlines until EOF
 * \return An error code
 */
enum error read_print_loop(struct cstream *cs)
{
    struct state s = { .cs = cs };
    vec_init(&s.last_token_str);

    while (true)
    {
        int tok = next_token(&s);

        if (tok == T_WORD)
            printf("\"%s\" ", vec_cstring(&s.last_token_str));
        else
            printf("%s ", TOKEN_STR(tok));

        if (tok == T_EOF)
            break;
    }

    vec_destroy(&s.last_token_str);

    return NO_ERROR;
}

int main(int argc, char *argv[])
{
    int rc;

    // Parse command line arguments and get an input stream
    struct cstream *cs;
    if ((cs = parse_args(argc, argv)) == NULL)
    {
        rc = 1;
        goto err_parse_args;
    }

    // Run the test loop
    if (read_print_loop(cs) != NO_ERROR)
    {
        rc = 1;
        goto err_loop;
    }

    // Success
    rc = 0;

err_loop:
    cstream_free(cs);
err_parse_args:
    return rc;
}
