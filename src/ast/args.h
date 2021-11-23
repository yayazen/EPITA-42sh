#pragma once

/**
 * The structure built by the main function and passed
 * to the parser.
 */
struct args
{
    /**
     * Print the AST as tdot
     *
     * When activated, the first command is parsed
     * and printed on the screen then the program
     * quit immediatly.
     */
    int print_ast;

    /**
     * Enable verbose mode
     */
    int verbose;

    /**
     * The input file descriptor
     */
    int fd;
};