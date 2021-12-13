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
     * \brief specify if a token shall be collected from the lexer
     */
    LEX_COLLECT = 1 << 4,

    /**
     * \brief specify if the next token is the first of a command
     */
    LEX_CMDSTART = 1 << 5,

    /**
     * \brief specify if the next token is the first of a command
     */
    PARSER_LINE_START = 1 << 6
};
