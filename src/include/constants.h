/**
 * \file constants.h
 * \brief Projet constants. These constants are used in the whole project,
 * especially as flags.
 *
 * These constants must all have a distinct value.
 */

#pragma once

#define PACKAGE "42sh"
#define VERSION "0.1.0"

enum
{
    /**
     * \brief show help and exit
     */
    OPT_HELP = 1 << 0,

    /**
     * \brief run lexer and print only tokens
     */
    OPT_DEBUG = 1 << 1,

    /**
     * \brief run parser and print ast tree
     */
    OPT_PRINT_AST = 1 << 2,

    /**
     * \brief run parser, print ast tree in dot format
     */
    OPT_PRINT_AST_DOT = 1 << 3,

    /**
     * \brief executing a script
     */
    MODE_SCRIPT = 1 << 4,

    /**
     * \brief executing commands specified as argument
     */
    MODE_COMMAND = 1 << 5,

    /**
     * \brief executing commands specified as argument
     */
    MODE_INPUT = 1 << 6,

    /**
     * \brief executing inside a function
     */
    IN_FUNCTION = 1 << 7,

    /**
     * \brief executing commands specified as argument
     */
    IS_INTERACTIVE = 1 << 8,

    /**
     * \brief specify if a token shall be collected from the lexer
     */
    LEX_COLLECT = 1 << 9,

    /**
     * \brief specify if the next token is the first of a command
     */
    LEX_CMDSTART = 1 << 10,

    /**
     * \brief specify if the next token is the first of a command
     */
    PARSER_LINE_START = 1 << 11
};

enum
{
    /**
     * \brief exit with main loop exit (should stop execution)
     */
    EXIT_WITH_LOOP_EXIT = 1 << 1,

    /**
     * \brief exit without main loop exit (should resume execution with next
     * line)
     */
    EXIT_WITHOUT_LOOP_EXIT = 1 << 2
};