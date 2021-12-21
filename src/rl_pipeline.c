#include <assert.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ctx.h"
#include "rule.h"
#include "token.h"

int rl_pipeline(struct rl_state *s)
{
    struct rl_exectree *node;
    int negate = 0;

    /* ['!'] */
    if (rl_accept(s, T_BANG) == true)
        negate = 1;
    else if (s->err != NO_ERROR)
        return -s->err;

    /* command */
    if (rl_cmd(s) <= 0)
        return -s->err;
    struct rl_exectree *child = s->node;
    if (!(node = rl_exectree_new(RL_PIPELINE)))
        return -(s->err = UNKNOWN_ERROR);
    node->attr.pipe.negate = negate;
    node->child = child;

    /* ('|' ('\n')* command)* */
    while (rl_accept(s, T_PIPE) == true)
    {
        while (rl_accept(s, T_LF) == true)
            ;

        if (rl_cmd(s) <= 0)
            break;

        child->sibling = s->node;
        child = child->sibling;
    }

    s->node = node;
    return (s->err != NO_ERROR) ? -s->err : true;
}

static inline int __piperun(const struct ctx *ctx, struct rl_exectree *rl_pipe)
{
    int fdin = STDIN_FILENO;

    struct attr_pipeline *p = &rl_pipe->attr.pipe;
    struct rl_exectree *node = rl_pipe->child;
    do
    {
        assert(node->type == RL_SIMPLE_CMD || node->type == RL_SHELL_CMD);

        if (pipe(p->fd) < 0)
            return EXECUTION_ERROR;

        /* Set the file descriptors to close on fork */
        fcntl(p->fd[0], F_SETFD, FD_CLOEXEC);
        fcntl(p->fd[1], F_SETFD, FD_CLOEXEC);

        node->attr.cmd.fd[0] = fdin;
        node->attr.cmd.fd[1] = (node->sibling) ? p->fd[1] : STDOUT_FILENO;

        rl_exec_cmd(ctx, node);

        close(p->fd[1]);
        fdin = p->fd[0];
    } while ((node = node->sibling));

    return NO_ERROR;
}

static inline int __pipewait(struct rl_exectree *rl_pipe)
{
    int status = 0;
    struct rl_exectree *node = rl_pipe->child;
    do
    {
        assert(node->type == RL_SIMPLE_CMD || node->type == RL_SHELL_CMD);

        if (node->attr.cmd.pid != -1)
        {
            waitpid(node->attr.cmd.pid, &(node->attr.cmd.status), 0);
            node->attr.cmd.status = WEXITSTATUS(node->attr.cmd.status);
        }

        status = node->attr.cmd.status;
        node->attr.cmd.pid = -1;
        node->attr.cmd.fd[0] = STDIN_FILENO;
        node->attr.cmd.fd[1] = STDOUT_FILENO;
        node->attr.cmd.fd[2] = STDERR_FILENO;
    } while ((node = node->sibling));

    return status;
}

int rl_exec_pipeline(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(node && node->child && node->type == RL_PIPELINE);

    int status = 0;
    __piperun(ctx, node);
    status = __pipewait(node);
    return node->attr.pipe.negate ? !status : status;
}
