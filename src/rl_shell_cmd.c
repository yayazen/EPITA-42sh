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

    /* rule_until */
    else if (rl_until(s) == true)
        ;

    else
        return false;

    if (!(node = rl_ast_new(RL_SHELL_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = s->ast;
    s->ast = node;
    return true;
}

static inline int __redirect(int oldfd, int newfd, int closefd)
{
    if (oldfd == newfd)
        return 0;
    if (dup2(oldfd, newfd) == -1)
        return -1;
    if (closefd && close(oldfd) == -1)
        return -1;
    return 0;
}

int rl_exec_shell_cmd(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_SHELL_CMD);

    ast->pid = fork();
    assert(ast->pid != -1);

    if (ast->pid == 0)
    {
        assert(__redirect(ast->fd[0], STDIN_FILENO, true) == 0);
        assert(__redirect(ast->fd[1], STDOUT_FILENO, false) == 0);
        switch (ast->child->type)
        {
        case RL_COMPOUND_LIST:
            exit(rl_exec_compound_list(ast->child));
        case RL_IF:
            exit(rl_exec_if_clause(ast->child));
        case RL_WHILE:
            exit(rl_exec_while(ast->child));
        case RL_UNTIL:
            exit(rl_exec_until(ast->child));
        default:
            exit(42);
        }
    }
    return 0;
}
