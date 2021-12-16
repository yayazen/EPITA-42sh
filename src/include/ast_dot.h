#pragma once

#include "rule.h"

/**
 * \brief Print built AST as dot file
 * \param ast The AST to print
 */
void ast_dot_print(struct rl_exectree *node);