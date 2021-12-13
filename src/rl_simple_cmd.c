#include <assert.h>
#include <unistd.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_simple_cmd(struct rl_state *s)
{
    struct rl_ast *node;

    /* WORD */
    // s->flag |= LEX_CMDSTART;
    if (rl_accept(s, T_WORD, RL_WORD) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_SIMPLE_CMD)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* WORD* */
    // s->flag &= ~LEX_CMDSTART;
    while (rl_accept(s, T_WORD, RL_WORD) == true)
    {
        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : 1;
}

int rl_exec_simple_cmd(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_SIMPLE_CMD);

    char *argv[10] = { 0 };
    argv[0] = (ast = ast->child)->word;
    for (int i = 1; (ast = ast->sibling) != NULL; i++)
        argv[i] = ast->word;

    return execvp(argv[0], argv);
}
