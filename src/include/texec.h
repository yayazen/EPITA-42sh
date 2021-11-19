#pragma once

#include "tast.h"

/**
 * This structure holds all the information that
 * are used alongside the AST to execute the programs
 */
struct texec
{
    // Environment variables
    // Current working directory
    // Last exit code
};

/**
 * Execute a command entered by a user / a script
 *
 * This function might be called
 * recursively in a future implementation
 *
 * @param exec The execution environment. This structure
 * might be updated by the executed program (for example by cd, export, ...)
 *
 * @param ast The AST tree to execute
 *
 * @returns The function return 1 if the execution
 * panicked and the program should exit.
 */
int exec_command(struct texec *exec, struct tast *ast);