#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>

#include "lexer_dfa.h"
#include "token.h"

#define DFA(C, S) dfa_eval((C), (S))
#define DFA_TERM(S) dfa_term((S))
#define DFA_TOKEN(S) dfa_token((S))

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
static inline int _is_meta(int c)
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
    int t = T_WORD;
    int c;
    int rc;

    vec_reset(word);
    __eat_whitespaces(cs);
    while ((rc = cstream_peek(cs, &c)) == NO_ERROR && c != EOF)
    {
        cs->line_start = false;
        s = DFA(c, s);

        if (s == DFA_ERR_STATE)
        {
            if (TOKEN_TYPE(t) == SPECIAL || _is_meta(c))
                break;
            int ns = DFA(c, DFA_ENTRY_STATE);
            s = (ns != DFA_ERR_STATE) ? ns : DFA_ENTRY_STATE;
            t = T_WORD;
        }
        else if (t == T_WORD && word->size > 0 && s != DFA(c, s) && _is_meta(c))
            break;
        else if (DFA_TERM(s))
        {
            t = DFA_TOKEN(s);
            if (TOKEN_TYPE(t) == KEYWORD && !(flag & LEX_LINE_START))
                t = T_WORD;
        }

        vec_push(word, c);
        if ((rc = cstream_pop(cs, &c)) != NO_ERROR || t == T_LF)
            break;
    }

    cs->line_start = true;

    return (rc != NO_ERROR) ? -rc : (c == EOF && !word->size) ? T_EOF : t;
}
