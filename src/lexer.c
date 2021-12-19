#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>
#include <stdio.h>
#include <utils/vec.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

#define DIGIT(N) ('0' <= (N) && (N) <= '9')
#define ABS(N) (((N) < 0) ? -(N) : (N))

#define DFA(C, S) (dfa_eval((C), (S)))
#define DFA_TERM(S) (dfa_term((S)))
#define DFA_TOKEN(S) (dfa_token((S)))

#define LEX_MODE_SQUOTE (1 << 1)
#define LEX_MODE_DQUOTE (1 << 2)
#define LEX_MODE_BACKSLASH (1 << 3)
#define LEX_MODE_DOLLAR (1 << 4)
#define LEX_MODE_CMD_SUB (1 << 5)
#define LEX_MODE_PARAM_EXP (1 << 6)
#define LEX_MODE_ARITH_EXP (1 << 7)

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

/**
 * \brief set mode for the lexer depending on `c`
 */
static inline int __lexmode(int c, int mode)
{
    if (mode & LEX_MODE_BACKSLASH)
        return mode & ~LEX_MODE_BACKSLASH;

    else if (c == '\\' && !(mode & LEX_MODE_SQUOTE))
        mode |= LEX_MODE_BACKSLASH;

    else if (c == '\'' && !(mode & LEX_MODE_DQUOTE))
        mode ^= LEX_MODE_SQUOTE;

    else if (c == '"' && !(mode & LEX_MODE_SQUOTE))
        mode ^= LEX_MODE_DQUOTE;

    else if (c == '`' && !(mode & LEX_MODE_SQUOTE))
        mode ^= LEX_MODE_CMD_SUB;

    else if ((c == '(' && mode & LEX_MODE_DOLLAR)
             || (c == ')' && mode & LEX_MODE_CMD_SUB))
        mode ^= LEX_MODE_CMD_SUB;

    else if ((c == '{' && mode & LEX_MODE_DOLLAR)
             || (c == '}' && mode & LEX_MODE_PARAM_EXP))
        mode ^= LEX_MODE_PARAM_EXP;

    else if (c == '$' && !(mode & LEX_MODE_SQUOTE))
        return mode | LEX_MODE_DOLLAR;

    return mode & ~LEX_MODE_DOLLAR;
}

static inline int __evalword(struct rl_state *rls)
{
    int c;
    if ((rls->err = cstream_peek(rls->cs, &c)) != NO_ERROR)
        return rls->err;

    if (rls->token != T_WORD)
        return rls->err;
    else if (rls->word.size == 1 && DIGIT(rls->word.data[0])
             && (c == '<' || c == '>'))
        rls->token = T_IONUMBER;
    else if (rls->flag & LEX_CMDSTART && strchr(vec_cstring(&rls->word), '='))
        rls->token = T_ASSIGN_WORD;

    return rls->err;
}

/* Recursively collect a token */
static int __lexaux(struct rl_state *rls, int state, int mode)
{
    int c;
    if ((rls->err = cstream_peek(rls->cs, &c)) != NO_ERROR || c == EOF)
        return rls->err;

    state = DFA(c, state);

    if (state == DFA_ERR_STATE || rls->token == T_WORD)
    {
        if (TOKEN_TYPE(rls->token) == SPECIAL
            || (!mode && strchr(TOKEN_DELIM, c)))
            return rls->err;

        rls->token = T_WORD;
        state = DFA_ENTRY_STATE;
    }
    else if (DFA_TERM(state))
    {
        rls->token = DFA_TOKEN(state);

        if (TOKEN_TYPE(rls->token) == KEYWORD && !(rls->flag & LEX_CMDSTART))
            rls->token = T_WORD;
    }

    vec_push(&rls->word, c);
    rls->cs->line_start = !mode && !(rls->flag & PARSER_LINE_START);

    if ((rls->err = cstream_pop(rls->cs, &c)) != NO_ERROR || rls->token == T_LF)
        return rls->err;

    int submode;
    if ((submode = __lexmode(c, mode)) != mode)
    {
        /* lexer is leving a mode */
        if (!submode)
            return rls->err;

        /* lexer is entering a new sub mode */
        mode &= ~LEX_MODE_DOLLAR;
        __lexaux(rls, state, submode);
    }

    return __lexaux(rls, state, mode);
}

int lexer(struct rl_state *rls)
{
    __eatwhitespaces(rls->cs);
    vec_reset(&rls->word);
    rls->token = T_EOF;

    rls->err = __lexaux(rls, DFA_ENTRY_STATE, 0);
    if (rls->token == T_WORD)
        __evalword(rls);

    return rls->err;
}
