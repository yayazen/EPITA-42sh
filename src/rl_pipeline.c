#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_pipeline(struct rl_state *s)
{
    struct rl_ast *node;

    /* ['!'] */
    if (rl_accept(s, T_BANG, RL_NORULE) < 0)
        return -s->err;

    /* command */
    if (rl_cmd(s) <= 0)
        return -s->err;
    struct rl_ast *child = s->ast;
    if (!(node = rl_ast_new(RL_PIPELINE)))
        return -(s->err = UNKNOWN_ERROR);
    node->child = child;

    /* ('|' ('\n')* command)* */
    while (rl_accept(s, T_PIPE, RL_NORULE) == true)
    {
        while (rl_accept(s, T_LF, RL_NORULE) == true)
            ;

        if (rl_cmd(s) <= 0)
            break;

        child->sibling = s->ast;
        child = child->sibling;
    }

    s->ast = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

static inline int __redirect(int oldfd, int newfd)
{
    if (oldfd == newfd)
        return 0;

    return dup2(oldfd, newfd) != -1 && close(oldfd) != -1;
}

int rl_exec_pipeline(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_PIPELINE);

    int status;
    int fdin = STDIN_FILENO;

    int fd[2];
    ast = ast->child;
    do
    {
        pid_t pid;

        if (pipe(fd) < 0)
            return EXECUTION_ERROR;

        pid = fork();
        if (pid == 0)
        {
            dup2(fdin, 0);
            if (ast->sibling)
                dup2(fd[1], 1);
            close(fd[0]);
            rl_exec_cmd(ast);
            fprintf(stderr, PACKAGE ": %s: command not found...\n",
                    ast->child->word);
            exit(127);
        }
        else
        {
            // very bad stuff
            waitpid(pid, &status, 0);
            close(fd[1]);
            fdin = fd[0];
        }
    } while ((ast = ast->sibling));

    return WEXITSTATUS(status);
}
