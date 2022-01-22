/**
 * \file parser.h
 * \brief Our parser & AST executor root function.
 */

#pragma once

#include <io/cstream.h>

#include "symtab.h"

/** \brief Argument passed to the function in charge of running the parser &
executing the built tree.**/
struct parser_args
{
    /** \brief stream structure used for parser invocation */
    struct cstream *cs;
    /** \brief Flags customizing parser behavior */
    int flags;
    /** \brief Where last exit status should be stored */
    int *exit_status;
    /** \brief Global symbols table to use */
    struct symtab *symtab;
    /** \brief Number of program arguments */
    int program_args_count;
    /** \brief Pointer on the first program argument */
    char **program_args;
};

/** \brief print debug ast */
void print_debug_ast(struct rl_exectree *node);

/*!
 * \brief parser entry function
 * \param args Arguments passed to parser
 * \return zero or a negative value in case of error
 */
int parser(const struct parser_args *args);
