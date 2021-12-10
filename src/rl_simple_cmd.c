#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "rule.h"
#include "token.h"

int rl_simple_cmd(struct rl_state *s)
{
    int rc;
    struct rl_ast *node;
    if ((rc = rl_expect(s, T_WORD, RL_WORD)) <= 0)
        return rc;

    node = calloc(1, sizeof(struct rl_ast));
    node->type = RL_SIMPLE_CMD;
    node->child = s->ast;

    struct rl_ast *n = node->child;
    while ((rc = rl_accept(s, T_WORD, RL_WORD)) == 1)
    {
        n->sibling = s->ast;
        n = n->sibling;
    }

    s->ast = node;
    return (rc < 0) ? rc : 1;
}

int rl_exec_simple_cmd(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_SIMPLE_CMD);

    char *argv[10];
    ast = ast->child;
    argv[0] = ast->word;

    int i = 1;
    while ((ast = ast->sibling))
    {
        argv[i++] = ast->word;
    }
    argv[i] = NULL;

    if (fork() == 0)
        execvp(argv[0], argv);
    else
        wait(0);

    return 0;
}
