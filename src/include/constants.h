#pragma once

enum
{
    /**
     * \brief show help and exit
     */
    OPT_HELP = (1 << 0),

    /**
     * \brief run lexer and print only tokens
     */
    OPT_DEBUG = (1 << 1),

    /**
     * \brief run parser and print ast tree
     */
    OPT_PRINT_AST = (1 << 2),

    /**
     * \brief run parser, print ast tree in dot format
     */
    OPT_PRINT_AST_DOT = (1 << 3),

    /**
     * \brief specify if last token from lexer was consumed or not
     */
    LAST_TOKEN_EATEN = (1 << 4),

};
