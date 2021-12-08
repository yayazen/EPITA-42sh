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

#define IS_DIGIT(X) ('0' <= (X) && (X) <= '9')

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

/*
 * \brief continue an IONUMBER or WORD
 *        and set state for next iteration
 */
static inline int __reset_state(int c, int *t)
{
    *t = (*t == T_IONUMBER && IS_DIGIT(c)) ? T_IONUMBER : T_WORD;
    int ns = DFA(c, DFA_ENTRY_STATE);
    return (ns != DFA_ERR_STATE) ? ns : DFA_ENTRY_STATE;
}

/*
 * \brief true if `c` interrupt a word in state `s`
 */
static inline int __break_word(int c, int s)
{
    return s != DFA(c, s) && __is_meta(c);
}

int cs_lex(struct cstream *cs, struct vec *word, int flag)
{
    int s = DFA_ENTRY_STATE;
    int t = T_WORD;
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
            s = __reset_state(c, &t);
        }

        else if (t == T_WORD && word->size && __break_word(c, s))
            break;

        else if (DFA_TERM(s) && (t != T_WORD || word->size == 0))
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

    return (rc != NO_ERROR) ? -rc : (c == EOF && !word->size) ? T_EOF : t;
}
