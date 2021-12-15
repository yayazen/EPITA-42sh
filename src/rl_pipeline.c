#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>

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

int rl_exec_pipeline(struct rl_ast *ast)
{
    assert(ast && ast->child && ast->type == RL_PIPELINE);

    int status = 0;
    int fdin = STDIN_FILENO;
    int fd[2];
    struct rl_ast *cmd;

    cmd = ast->child;
    while (cmd)
    {
        if (pipe(fd) < 0)
            return -EXECUTION_ERROR;
        cmd->fd[0] = fdin;
        cmd->fd[1] = (cmd->sibling) ? fd[1] : STDOUT_FILENO;
        int ret = rl_exec_cmd(cmd);
        if (cmd->pid == -1)
            status = ret;
        close(fd[1]);
        fdin = fd[0];
        cmd = cmd->sibling;
    }

    cmd = ast->child;
    while (cmd)
    {
        if (cmd->pid != -1)
        {
            if (cmd->sibling)
                waitpid(cmd->pid, NULL, 0);
            else
            {
                waitpid(cmd->pid, &status, 0);
                status = WEXITSTATUS(status);
            }
        }

        cmd->fd[0] = STDIN_FILENO;
        cmd->fd[1] = STDOUT_FILENO;
        cmd->pid = -1;
        cmd = cmd->sibling;
    }

    return status;
}
