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
    struct rl_ast *cmd = ast->child;
    do
    {
        if (pipe(fd) < 0)
            return EXECUTION_ERROR;

        cmd->fd[0] = fdin;
        cmd->fd[1] = (cmd->sibling) ? fd[1] : STDOUT_FILENO;
        rl_exec_cmd(cmd);
        close(fd[1]);
        fdin = fd[0];

    } while ((cmd = cmd->sibling));

    cmd = ast->child;
    do
    {
        waitpid(cmd->pid, &status, 0);
    } while ((cmd = cmd->sibling));

    return WEXITSTATUS(status);
}
