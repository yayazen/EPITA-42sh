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
    else if (rl_accept(s, T_LESSAND) == true)
        return T_LESSAND;
    return -1;
}

int rl_redirection(struct rl_state *s)
{
    int token;
    int ionumber = STDOUT_FILENO;
    struct rl_exectree *node;

    /* [IONUMBER] */
    if (rl_accept(s, T_IONUMBER) == true)
        ionumber = atoi(vec_cstring(&s->word));

    /* ">", "<", ... */
    if ((token = __redirtoken(s)) == -1)
        return false;

    /* WORD */
    if (rl_expect(s, T_WORD) <= 0)
        return -s->err;
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
    if (redir->token == T_GREAT)
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (redir->token == T_DGREAT)
        flags = O_WRONLY | O_APPEND;

    int fd = open(redir->file, flags, DEFAULT_MODE);
    if (fd == -1 || __redirect(fd, redir->ionumber, true) != 0)
        return EXECUTION_ERROR;

    return NO_ERROR;
}
