#pragma once

#include "ast.h"

/**
 * Print the AST as dot format.
 *
 * This function is really useful for
 * debugging, and is used when the argument
 * --print-ast is passed through the command line
 */
void print_ast(struct ast *ast);