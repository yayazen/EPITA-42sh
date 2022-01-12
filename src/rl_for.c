#include <assert.h>
#include <setjmp.h>

#include "constants.h"
#include "ctx.h"
#include "rule.h"
#include "symexp.h"
#include "token.h"

/** \brief eat next new lines in lexer stream */
static bool __eat_new_lines(struct rl_state *s)
{
    /* ('\n')* */
    while (rl_accept(s, T_LF) > 0)
        ;
    return true;
}

int rl_for(struct rl_state *s)
{
    struct rl_exectree *node;

    /* For WORD */
    if (rl_accept(s, T_FOR) <= 0 || rl_expect(s, T_WORD) <= 0)
        return -s->err;

    assert(node = rl_exectree_new(RL_FOR));
    node->attr.word = strdup(vec_cstring(&s->word));

    struct rl_exectree *words = NULL;
    struct rl_exectree *tail = NULL;

    s->flag |= LEX_CMDSTART;

    /* [';'] */
    if (rl_accept(s, T_SEMICOL) == true)
        ;

    /* ('\n')* 'in' */
    else if (__eat_new_lines(s) && rl_accept(s, T_IN) == true)
    {
        /* (WORD)* */
        while (rl_accept(s, T_WORD) == true)
        {
            /* Add the word to the end of the "linked list" */
            struct rl_exectree *next = rl_exectree_new(RL_WORD);
            assert(next != NULL);
            next->attr.word = strdup(vec_cstring(&s->word));

            /* First element */
            if (words == NULL || tail == NULL)
            {
                words = next;
                tail = next;
            }

            /* Next elements */
            else
            {
                tail->sibling = next;
                tail = next;
            }
        }

        /* (';'|'\n') */
        if (rl_accept(s, T_SEMICOL) <= 0 && rl_expect(s, T_LF) <= 0)
        {
            goto err_parsing;
        }
    }

    /* ('\n')* */
    __eat_new_lines(s);

    /* do_group */
    s->flag |= PARSER_LINE_START;
    if (rl_do_group(s) <= 0)
    {
        goto err_parsing;
    }

    /* Error handling */
    if (false)
    {
    err_parsing:
        s->flag &= ~PARSER_LINE_START;
        rl_exectree_free(words);
        rl_exectree_free(node);

        return -s->err;
    }

    s->flag &= ~PARSER_LINE_START;

    /* In order to ease execution, we store first the compound list to execute
     * and then the words */
    node->child = s->node; /* the compound list */
    node->child->sibling = words; /* the list of words */
    s->node = node;

    return true;
}

int rl_exec_for(const struct ctx *ctx, struct rl_exectree *node)
{
    assert(ctx && node && node->type == RL_FOR);
    assert(node->attr.word);
    assert(node->child->type == RL_COMPOUND_LIST);

    struct rl_exectree *child = node->child;
    struct rl_exectree *curr_word = child;

    // Perform expansion
    struct list *words = list_new(10);
    while ((curr_word = curr_word->sibling))
    {
        symexp_word(ctx, curr_word->attr.word, words);
    }

    // Register the generated list to automatically cleanup memory
    // in case of memory jump above this execution point
    CTX_CHILD_FOR_LIST(ctx, list_ctx, words);

    volatile int status = 0;
    volatile int val;

    jmp_buf jump_buffer;
    struct ctx_jmp jmp_node;

    for (volatile unsigned int i = 0; i < words->size; i++)
    {
        /* Update the word in the list */
        assert(symtab_add(ctx->st, node->attr.word, KV_WORD,
                          strdup(words->data[i])));

        val = setjmp(jump_buffer);

        if (val == JMP_CONTINUE)
        {
            continue;
        }
        else if (val == JMP_BREAK)
        {
            break;
        }
        else if (val == JMP_NOOP)
        {
            struct ctx child_ctx =
                ctx_add_jump(&list_ctx, &jmp_node, &jump_buffer);
            status = rl_exec_compound_list(&child_ctx, child);
        }
        else
            assert(0);
    }

    list_free(words);

    return status;
}