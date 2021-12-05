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
    struct vec word;
    int state = DFA_ENTRY_STATE;
    int t = -1;
    int c;
    int rc;

    vec_init(&word);
    __eat_whitespaces(cs);
    while (t != T_LF && (rc = cstream_peek(cs, &c)) == NO_ERROR && c != EOF)
    {
        int ns = dfa_eval(c, state);
        if (ns == DFA_ERR_STATE || t == T_WORD)
        {
            if (t != -1 && (TOKEN_TYPE(t) == SPECIAL || __is_meta(c)))
                break;
            t = T_WORD;
        }
        else if (dfa_term(ns))
        {
            t = dfa_token(ns);
            if (TOKEN_TYPE(t) != SPECIAL && !(flag & LEXER_LINE_START))
                t = T_WORD;
        }

        state = (ns != DFA_ERR_STATE) ? ns : DFA_ENTRY_STATE;
        vec_push(&word, c);
        if ((rc = cstream_pop(cs, &c)) != NO_ERROR)
            break;
    }

    token->key = t;
    token->str = vec_cstring(&word);
    return rc;
}
