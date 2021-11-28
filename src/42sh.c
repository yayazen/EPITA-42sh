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
    int option_index = 0;
    struct cstream *stream = NULL;

    // Parse arguments
    struct option options[] = { { "command", required_argument, 0, 'c' },
                                { 0, 0, 0, 0 } };

    while (!err)
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
            if (stream == NULL)
                stream = cstream_string_create(optarg);
            break;

        default:
            err = 1;
        }
    }

    // Open file stream only in case of success
    if (optind < argc && !err && stream == NULL)
    {
        // Handle non-option argv elements
        FILE *fp = fopen(argv[optind++], "r");
        if (fp == NULL)
        {
            warn("failed to open input file");
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
enum error read_print_loop(struct cstream *cs, struct vec *line)
{
    enum error err;

    while (true)
    {
        // Read the next character
        int c;
        if ((err = cstream_pop(cs, &c)))
            return err;

        // If the end of file was reached, stop right there
        if (c == EOF)
        {
            if (line->size > 0)
                printf(">> last line data: %s\n", vec_cstring(line));
            break;
        }

        // If a newline was met, print the line
        if (c == '\n')
        {
            printf(">> line data: %s\n", vec_cstring(line));

            vec_reset(line);
            continue;
        }

        // Otherwise, add the character to the line
        vec_push(line, c);
    }

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

    // Create a vector to hold the current line
    struct vec line;
    vec_init(&line);

    // Run the test loop
    if (read_print_loop(cs, &line) != NO_ERROR)
    {
        rc = 1;
        goto err_loop;
    }

    // Success
    rc = 0;

err_loop:
    cstream_free(cs);
    vec_destroy(&line);
err_parse_args:
    return rc;
}
