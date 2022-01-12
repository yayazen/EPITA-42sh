#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/vec.h>

#include "rule.h"
#include "token.h"

#define DEFAULT_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH

static int __redirtoken(struct rl_state *s)
{
    /* if IONUMBER was matched, this shall always hit */
    /* ">" */
    if (rl_accept(s, T_GREAT) == true)
        return T_GREAT;
    /* "<" */
    else if (rl_accept(s, T_LESS) == true)
        return T_LESS;
    /* ">>" */
    else if (rl_accept(s, T_DGREAT) == true)
        return T_DGREAT;
    /* ">&" */
    else if (rl_accept(s, T_GREATAND) == true)
        return T_GREATAND;
    /* "<&" */
    else if (rl_accept(s, T_LESSAND) == true)
        return T_LESSAND;
    /* ">|" */
    else if (rl_accept(s, T_CLOBBER) == true)
        return T_CLOBBER;
    /* "<>" */
    else if (rl_accept(s, T_LESSGREAT) == true)
        return T_LESSGREAT;
    return -1;
}

int rl_redirection(struct rl_state *s)
{
    int token;
    int ionumber = STDOUT_FILENO;
    struct rl_exectree *node;
    int got_io_number = 0;

    /* [IONUMBER] */
    if (rl_accept(s, T_IONUMBER) == true)
    {
        ionumber = atoi(vec_cstring(&s->word));
        got_io_number = 1;
    }

    /* ">", "<", ... */
    if ((token = __redirtoken(s)) == -1)
        return false;

    /* overwrite stdin if required */
    if (!got_io_number
        && (token == T_LESS || token == T_LESSAND || token == T_LESSGREAT))
        ionumber = STDIN_FILENO;

    /* IONUMBER | WORD */
    if (!((token == T_LESSAND || token == T_GREATAND)
          && rl_accept(s, T_IONUMBER) == true)
        && (rl_expect(s, T_WORD) <= 0))
        return -(s->err = PARSER_ERROR);

    node = rl_exectree_new(RL_REDIRECTION);
    if (!node || !(node->attr.redir.file = strdup(vec_cstring(&s->word))))
        return -(s->err = PARSER_ERROR);

    node->attr.redir.ionumber = ionumber;
    node->attr.redir.token = token;
    s->node = node;
    return 1;
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

int rl_exec_redirection(struct rl_exectree *node)
{
    assert(node && node->type == RL_REDIRECTION);

    struct attr_redir *redir = &node->attr.redir;
    if (redir->ionumber > 2)
        return NO_ERROR;

    int flags = 0;
    int fd = -1;

    /* > | >| */
    if (redir->token == T_GREAT || redir->token == T_CLOBBER)
        flags = O_WRONLY | O_CREAT | O_TRUNC;

    /* < */
    else if (redir->token == T_LESS)
        flags = O_RDONLY;

    /* <> */
    else if (redir->token == T_LESSGREAT)
        flags = O_RDWR;

    /* >> */
    else if (redir->token == T_DGREAT)
        flags = O_WRONLY | O_APPEND | O_CREAT;

    /* >& | <& => reuse file descriptors */
    else if (redir->token == T_GREATAND || redir->token == T_LESSAND)
        fd = atoi(redir->file);

    /* unsupported case */
    else
        assert(0);

    fd = fd > 0 ? fd : open(redir->file, flags, DEFAULT_MODE);
    if (fd == -1
        || __redirect(fd, redir->ionumber, flags == 0 ? false : true) != 0)
    {
        return EXECUTION_ERROR;
    }

    return NO_ERROR;
}
