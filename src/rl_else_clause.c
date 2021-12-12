#include <assert.h>

#include "rule.h"
#include "token.h"

/**
 * \brief Handles else clause
 */
int __rl_else(struct rl_state *s)
{
    /* else */
    if (rl_accept(s, T_ELSE, RL_NORULE) <= 0)
        return -s->err;

    /* compound_list */
    if (rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_ast *node = calloc(1, sizeof(struct rl_ast));
    assert(node != NULL);

    node->type = RL_ELSE;
    node->child = s->ast;

    s->ast = node;
    return true;
}

/**
 * \brief Handles elif clause
 */
int __rl_elif(struct rl_state *s)
{
    /* elif compound_list */
    if (rl_accept(s, T_ELIF, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
        return -s->err;

    struct rl_ast *node = calloc(1, sizeof(struct rl_ast));
    assert(node != NULL);

    node->type = RL_ELIF;
    node->child = s->ast;
    s->ast = NULL;

    /* then compound_list*/
    if (rl_expect(s, T_THEN, RL_NORULE) <= 0 || rl_compound_list(s) <= 0)
    {
        rl_ast_free(node);
        return -s->err;
    }

    node->child->sibling = s->ast;
    node->child->sibling->sibling = NULL;
    s->ast = NULL;

    /* [else_clause] */
    if (rl_else_clause(s) == true)
    {
        node->child->sibling->sibling = s->ast;
    }

    s->ast = node;
    return true;
}

int rl_else_clause(struct rl_state *s)
{
    return __rl_else(s) == true ? true : __rl_elif(s);
}

int rl_exec_else_clause(struct rl_ast *ast)
{
    assert(ast && (ast->type == RL_ELIF || ast->type == RL_ELSE));

    /* else */
    if (ast->type == RL_ELSE)
        return rl_exec_compound_list(ast->child);

    /* elif */
    int status = rl_exec_compound_list(ast->child);

    // If the `elif` condition is met
    if (status == 0)
    {
        status = rl_exec_compound_list(ast->child->sibling);
    }

    // Otherwise, run `else clause` (if available)
    else if (ast->child->sibling->sibling != NULL)
    {
        status = rl_exec_else_clause(ast->child->sibling->sibling);
    }

    else
    {
        status = 0;
    }

    return status;
}