#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "quotes.h"
#include "rule.h"
#include "token.h"

int rl_simple_cmd(struct rl_state *s)
{
    struct rl_ast *node;

    /* WORD */
    if (rl_accept(s, T_WORD, RL_WORD) <= 0)
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
    argv[0] = expand_simple_quotes(ast->word);

    int i = 1;
    while ((ast = ast->sibling))
        argv[i++] = expand_simple_quotes(ast->word);
    argv[i] = NULL;

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp(argv[0], argv);

        // If the command was not found, we have to return
        // exit status 127
        exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}
