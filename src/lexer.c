#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>
#include <stdio.h>

#include "lexer_dfa.h"
#include "token.h"

/**
 * \brief    move `stream` to the next non-blank character
 * \brief stream
 */
static inline struct cstream *__eat_whitespaces(struct cstream *cs)
{
    int c;
    while (cstream_peek(cs, &c) == NO_ERROR && (c == ' ' || c == '\t'))
        assert(cstream_pop(cs, &c) == NO_ERROR);
    return cs;
}

/*
 * \brief  True if `c` is a meta character
 *         i.e a one char token marked as SPECIAL.
 * \return boolean
 */
static inline int __is_meta(int c)
{
    int state = dfa_eval(c, DFA_ENTRY_STATE);
    return state != DFA_ERR_STATE && dfa_term(state)
        && TOKEN_TYPE(dfa_token(state)) == SPECIAL;
}

int cs_lex(struct cstream *cs, struct token *token, int flag)
{
    int state = DFA_ENTRY_STATE;
    int rc;
    int c;
    struct vec word;

    vec_init(&word);
    token->key = EOF;
    __eat_whitespaces(cs);
    while ((rc = cstream_peek(cs, &c)) == NO_ERROR && c != EOF)
    {
        state = dfa_eval(c, state);
        if (state == DFA_ERR_STATE)
        {
            if (token->key != EOF && TOKEN_TYPE(token->key) == SPECIAL)
                break;
            token->key = T_WORD;
            state = DFA_ENTRY_STATE;
        }

        if (token->key == T_WORD && state != dfa_eval(c, state) && __is_meta(c))
            break;
        else if (dfa_term(state))
        {
            token->key = dfa_token(state);
            if (TOKEN_TYPE(token->key) != SPECIAL && !(flag & LEX_LINE_START))
                token->key = T_WORD;
        }

        vec_push(&word, c);
        if ((rc = cstream_pop(cs, &c)) != NO_ERROR || token->key == T_LF)
            break;
    }

    token->str = vec_cstring(&word);
    return rc;
}
