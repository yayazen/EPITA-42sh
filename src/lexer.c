#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>
#include <utils/vec.h>

#include "lexer_dfa.h"
#include "token.h"

#define CMDSTART 1

#define DFA(C, S) (dfa_eval((C), (S)))
#define DFA_TERM(S) (dfa_term((S)))
#define DFA_TOKEN(S) (dfa_token((S)))

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
 */
static inline int __is_meta(int c)
{
    int s = DFA(c, DFA_ENTRY_STATE);
    if (s != DFA_ERR_STATE && DFA_TERM(s)
        && TOKEN_TYPE(DFA_TOKEN(s)) == SPECIAL)
        return 1;
    return 0;
}

int cs_lex(struct cstream *cs, struct vec *word, int flag)
{
    int s = DFA_ENTRY_STATE;
    int t = T_EOF;
    int c;
    int rc;

    __eat_whitespaces(cs);
    vec_reset(word);

    while ((rc = cstream_peek(cs, &c)) == NO_ERROR && c != EOF)
    {
        s = DFA(c, s);
        if (s == DFA_ERR_STATE)
        {
            if (TOKEN_TYPE(t) == SPECIAL || __is_meta(c))
                break;
            int ns = DFA(c, DFA_ENTRY_STATE);
            s = (ns != DFA_ERR_STATE) ? ns : DFA_ENTRY_STATE;
            t = T_WORD;
        }

        if (t == T_WORD)
        {
            if (s != DFA(c, s) && __is_meta(c))
                break;
        }
        else if (DFA_TERM(s))
        {
            t = DFA_TOKEN(s);
            if (TOKEN_TYPE(t) == KEYWORD && !(flag & CMDSTART))
                t = T_WORD;
        }

        vec_push(word, c);
        cs->line_start = DFA('\n', s) != s;
        if ((rc = cstream_pop(cs, &c)) != NO_ERROR || t == T_LF)
            break;
    }

    return (rc != NO_ERROR) ? -rc : t;
}
