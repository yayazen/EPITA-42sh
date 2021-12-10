#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "rule.h"
#include "token.h"

int rl_simple_cmd(struct rl_state *s)
{
    int rc;
    struct rl_ast *node;
    struct rl_ast **pos;

    if (!(node = calloc(1, sizeof(struct rl_ast))))
        return -1;
    node->type = RL_SIMPLE_CMD;

    pos = &node->child;
    while ((rc = rl_accept(s, T_WORD, RL_WORD)) == 1)
    {
        *pos = s->ast;
        pos = &(*pos)->sibling;
    }

    s->ast = node;
    return (rc < 0) ? rc : 1;
}

int rl_exec_simple_cmd(struct rl_ast *ast)
{
    assert(ast && ast->type == RL_SIMPLE_CMD);
    if (!(ast = ast->child))
        return 0;

    char *pathname = ast->word;
    char *argv[10];
    argv[0] = pathname;

    int i = 1;
    while ((ast = ast->sibling))
    {
        argv[i++] = ast->word;
    }
    argv[i] = NULL;

    return execvp(pathname, argv);
}
