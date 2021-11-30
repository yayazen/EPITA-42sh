#include <ast/lexer.h>
#include <ast/token.h>
#include <err.h>
#include <getopt.h>
#include <io/cstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/vec.h>

enum RUN_MODE
{
    DEFAULT_MODE,
    LEXER_ONLY,
};

static int mode = DEFAULT_MODE;

/**
 * \brief Run getopt_long on command line arguments and take appropriate actions
 * \return Nothing
 */
static void run_getopt(int argc, char **argv, struct cstream **stream, int *err)
{
    int option_index = 0;

    struct option options[] = { { "command", required_argument, 0, 'c' },
                                { "lexer", no_argument, 0, 'l' },
                                { 0, 0, 0, 0 } };

    while (!*err)
    {
        int c = getopt_long(argc, argv, "c:l", options, &option_index);

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

        case 'l':
            mode = LEXER_ONLY;
            break;

        default:
            *err = 1;
        }
    }
}

/**
 * \brief Create a stream to read from stdin
 * \return A character stream
 */
static struct cstream *create_stdin_stream(void)
{
    if (isatty(STDIN_FILENO))
        return cstream_readline_create();
    return cstream_file_create(stdin, /* fclose_on_free */ false);
}

/**
 * \brief Parse the command line arguments
 * \return A character stream
 */
static struct cstream *parse_args(int argc, char *argv[])
{
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
        fprintf(stderr,
                "Usage: %s [-l] [-c COMMAND] [SCRIPT] [ARGUMENTS ...]\n",
                argv[0]);

    if (err && stream != NULL)
    {
        cstream_free(stream);
        stream = NULL;
    }

    // By default, read from stdin
    if (!err && stream == NULL)
    {
        return create_stdin_stream();
    }

    return stream;
}

/**
 * \brief Read and print lexer data
 * \return An error code
 */
enum error lexer_loop(struct cstream *cs)
{
    struct state s = { .cs = cs };
    vec_init(&s.last_token_str);

    printf("Starting in lexer-only mode...\n");

    while (true)
    {
        int tok = next_token(&s);

        if (tok == T_WORD)
            printf("\"%s\" ", vec_cstring(&s.last_token_str));
        else if (tok == T_LF)
            printf("\n");
        else
            printf("%s ", TOKEN_STR(tok));

        if (tok == T_EOF)
            break;
    }

    vec_destroy(&s.last_token_str);

    return NO_ERROR;
}

/**
 * \brief Main loop. Calls the parser until an error / an EOF event occur s
 * \return An error code
 */
enum error default_loop(struct cstream *cs)
{
    printf("TODO : run parser. You might want to use the -l option to test the "
           "lexer");

    return cs != NULL ? NO_ERROR : EXECUTION_ERROR;
}

/**
 * \brief Main function of the program
 */
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

    if (mode == LEXER_ONLY)
    {
        if (lexer_loop(cs) != NO_ERROR)
        {
            rc = 1;
            goto err_loop;
        }
    }
    else
    {
        if (default_loop(cs) != NO_ERROR)
        {
            rc = 1;
            goto err_loop;
        }
    }

    // Success
    rc = 0;

err_loop:
    cstream_free(cs);
err_parse_args:
    return rc;
}
