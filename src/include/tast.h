#pragma once

#include "targs.h"

/**
 * The AST tree built for each command parsed.
 *
 * This AST is built by the parsed and executed
 * then by the executor
 */

struct tast
{
    // TODO in team :)
};

/**
 * Start the parser
 *
 * The function is the main loop of the program.
 *
 * @param args The main arguments
 *
 * @returns The returns value is 1 if the program panicked.
 * The exit status is 0 otherise.
 */
int parse(struct targs *args);