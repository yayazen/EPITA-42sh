#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "rule.h"
#include "token.h"

int rl_simple_cmd(struct rl_state *s)
{
    struct rl_ast *node;

    /* WORD */
    if (rl_expect(s, T_WORD, RL_WORD) <= 0)
        return -s->err;
    if (!(node = calloc(1, sizeof(struct rl_ast))))
        return -(s->err = UNKNOWN_ERROR);
    node->type = RL_SIMPLE_CMD;
    node->child = s->ast;

    /* WORD* */
    struct rl_ast *child = node->child;
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
    assert(ast && ast->type == RL_SIMPLE_CMD);

    char *argv[10];
    ast = ast->child;
    argv[0] = ast->word;

    int i = 1;
    while ((ast = ast->sibling))
        argv[i++] = ast->word;
    argv[i] = NULL;

    if (fork() == 0)
        return execvp(argv[0], argv);
    wait(0);
    return 0;
}
