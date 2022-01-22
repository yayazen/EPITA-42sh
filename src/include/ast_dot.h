/**
 * \file ast_dot.h
 * \brief Print AST tree in dot format.
 *
 * Dot file can be generated using the following command:
 *
 * ```bash
 * build/42sh --print-ast-dot -c "YOUR_COMMAND"
 * ```
 */

#pragma once

#include "rule.h"

/**
 * \brief Print built AST as dot file
 * \param ast The AST to print
 */
void ast_dot_print(struct rl_exectree *node);