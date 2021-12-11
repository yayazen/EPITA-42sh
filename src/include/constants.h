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
     * \brief run parser and print tree
     */
    OPT_PRINT_AST = (1 << 2),

    /**
     * \brief specify if last token from lexer was consumed or not
     */
    LAST_TOKEN_EATEN = (1 << 3),

};
