#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_shell_cmd(struct rl_state *s)
{
    struct rl_ast *node;

    /* '{' compound_list '}' */
    if (rl_accept(s, T_LBRACE, RL_NORULE) == true)
    {
        s->flag |= PARSER_LINE_START;
        if (rl_compound_list(s) <= 0 || rl_expect(s, T_RBRACE, RL_NORULE) <= 0)
            return -s->err;
        s->flag &= ~PARSER_LINE_START;
    }
    /* '(' compound_list ')' */
    else if (rl_accept(s, T_LPAR, RL_NORULE) == true)
    {
        s->flag |= PARSER_LINE_START;
        if (rl_compound_list(s) <= 0 || rl_expect(s, T_RPAR, RL_NORULE) <= 0)
            return -s->err;
        s->flag &= ~PARSER_LINE_START;
    }

    /* rule_if */
    else if (rl_if_clause(s) == true)
        ;

    /* rule_while */
    else if (rl_while(s) == true)
        ;

    else
        return false;

    if (!(node = rl_ast_new(RL_SHELL_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = s->ast;
    s->ast = node;
    return true;
}

int rl_exec_shell_cmd(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_SHELL_CMD);

    ast = ast->child;
    switch (ast->type)
    {
    case RL_COMPOUND_LIST:
        return rl_exec_compound_list(ast);
    case RL_IF:
        return rl_exec_if_clause(ast);
    case RL_WHILE:
        return rl_exec_while(ast);
    default:
        assert(0);
    }
}
