#include <assert.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

int rl_fundec(struct rl_state *s)
{
    // This is the only part of the grammar which is LL(1)
    // WORD
    if (rl_accept(s, T_WORD) <= 0)
        return -s->err;

    s->flag &= ~LEX_CMDSTART;
    char *funcname = strdup(vec_cstring(&s->word));

    /* '(' */
    if (rl_accept(s, T_LPAR) <= 0)
    {
        // We need to push back on the stream the word
        // Swap current token in state and word
        rl_buffer_token(s);
        vec_reset(&s->word);
        vec_pushstr(&s->word, funcname);
        s->token = T_WORD;

        free(funcname);
        return false;
    }

    /* ')' */
    if (rl_expect(s, T_RPAR) <= 0)
    {
        free(funcname);
        s->err = PARSER_ERROR;
        return -s->err;
    }

    s->flag |= LEX_CMDSTART;

    /* ('\n')* */
    while (rl_accept(s, T_LF) == true)
        ;

    /* shell_command */
    if (rl_shell_cmd(s) <= 0)
    {
        free(funcname);
        return -s->err;
    }

    struct rl_exectree *node = rl_exectree_new(RL_FUNDEC);
    assert(node);

    // Save word
    node->child = rl_exectree_new(RL_WORD);
    assert(node->child);
    node->child->attr.word = funcname;

    // Save function body
    node->child->sibling = s->node;

    s->node = node;

    return (s->err != NO_ERROR) ? -s->err : true;
}

int rl_exec_fundec(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(ctx && node);
    // TODO : save the function
    return 0;
}