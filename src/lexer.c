#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>
#include <utils/vec.h>

#include "rule.h"
#include "token.h"

#define LEX_MODE_SQUOTE (1 << 0)
#define LEX_MODE_DQUOTE (1 << 1)
#define LEX_MODE_DOLLAR (1 << 2)
#define LEX_MODE_CMD_SUB (1 << 3)
#define LEX_MODE_PARAM_EXP (1 << 4)
#define LEX_MODE_ARITH_EXP (1 << 5)

#define DFA(C, S) (dfa_eval((C), (S)))
#define DFA_TERM(S) (dfa_term((S)))
#define DFA_TOKEN(S) (dfa_token((S)))

/**
 * \brief  move `stream` to the next non-blank character
 * \param  a cstream
 */
static inline struct cstream *__eatwhitespaces(struct cstream *cs)
{
    int c;
    while (cstream_peek(cs, &c) == NO_ERROR && (c == ' ' || c == '\t'))
        assert(cstream_pop(cs, &c) == NO_ERROR);
    return cs;
}

/*
 * \brief  true if `c` is a meta character
 *         i.e a one char token marked as SPECIAL.
 */
static inline int __ismeta(int c)
{
    int s = DFA(c, DFA_ENTRY_STATE);
    if (s != DFA_ERR_STATE && DFA_TERM(s)
        && TOKEN_TYPE(DFA_TOKEN(s)) == SPECIAL)
        return 1;
    return 0;
}

/*
 * \brief set mode for the lexer depending on `c`
 */
static inline int __lexmode(int mode, int c)
{
    static int oldc = -1;

    if (c == '\'' && !(mode & LEX_MODE_DQUOTE))
        mode ^= LEX_MODE_SQUOTE;

    if (c == '"' && !(mode & LEX_MODE_SQUOTE))
        mode ^= LEX_MODE_DQUOTE;

    if (c == '`' && !(mode & LEX_MODE_SQUOTE))
        mode ^= LEX_MODE_CMD_SUB;

    if (oldc == '$' && c == '(')
        mode |= LEX_MODE_CMD_SUB;
    if (c == ')' && (mode & LEX_MODE_PARAM_EXP))
        mode &= ~LEX_MODE_CMD_SUB;

    if (oldc == '$' && c == '{')
        mode |= LEX_MODE_PARAM_EXP;
    if (c == '}' && (mode & LEX_MODE_PARAM_EXP))
        mode &= ~LEX_MODE_PARAM_EXP;

    oldc = c;
    return mode;
}

/*
 * \brief recursively collect a token
 * \return NO_ERROR or an error code as deined in utils/error.h
 */
static int __lexer(struct rl_state *rls, int s, int mode)
{
    int c;
    if ((rls->err = cstream_peek(rls->cs, &c)) != NO_ERROR || c == EOF)
        return rls->err;

    s = DFA(c, s);
    if (s == DFA_ERR_STATE || rls->token == T_WORD)
    {
        if (TOKEN_TYPE(rls->token) == SPECIAL
            || (!(mode = __lexmode(mode, c)) && __ismeta(c)))
            return rls->err;
        s = DFA_ENTRY_STATE;
        rls->token = T_WORD;
    }
    else if (DFA_TERM(s))
    {
        rls->token = DFA_TOKEN(s);
        if (TOKEN_TYPE(rls->token) == KEYWORD && !(rls->flag & 1))
            rls->token = T_WORD;
    }

    vec_push(&rls->word, c);
    rls->cs->line_start = !mode;
    if ((rls->err = cstream_pop(rls->cs, &c)) != NO_ERROR || rls->token == T_LF)
        return rls->err;

    return __lexer(rls, s, mode);
}

int lexer(struct rl_state *rls)
{
    rls->token = T_EOF;
    vec_reset(&rls->word);
    __eatwhitespaces(rls->cs);

    return __lexer(rls, DFA_ENTRY_STATE, 0);
}
