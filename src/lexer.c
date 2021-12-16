#include "lexer.h"

#include <assert.h>
#include <io/cstream.h>
#include <utils/vec.h>

#include "constants.h"
#include "rule.h"
#include "token.h"

#define LEX_MODE_SQUOTE (1 << 1)
#define LEX_MODE_DQUOTE (1 << 2)
#define LEX_MODE_BACKSLASH (1 << 3)
#define LEX_MODE_DOLLAR (1 << 4)
#define LEX_MODE_CMD_SUB (1 << 5)
#define LEX_MODE_PARAM_EXP (1 << 6)
#define LEX_MODE_ARITH_EXP (1 << 7)

#define DIGIT(N) ('0' <= (N) && (N) <= '9')

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



/**
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

/**
 * \brief true if `c` start a redirection token
 */
static inline int __isredir(int c)
{
    int s = DFA(c, DFA_ENTRY_STATE);
    if (s == DFA_ERR_STATE)
        return 0;
    return DFA_TERM(s) && (DFA_TOKEN(s) == T_LESS || DFA_TOKEN(s) == T_GREAT);
}

/**
 * \brief return true if `c`
 */

/**
 * \brief set mode for the lexer depending on `c`
 */
static inline int __lexmode(int mode, int c)
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

/**
 * \brief recursively collect a token
 * \return NO_ERROR or an error code as defined in utils/error.h
 */
static int __lexer(struct rl_state *rls, int s, int mode)
{
    int c;
    if ((rls->err = cstream_peek(rls->cs, &c)) != NO_ERROR || c == EOF)
        return rls->err;

    s = DFA(c, s);

    if (s == DFA_ERR_STATE || TOKEN_TYPE(rls->token) == DEFAULT)
    {
        s = DFA_ENTRY_STATE;
        mode = __lexmode(mode, c);
        if (rls->token == T_EOF || (rls->token == T_IONUMBER && !__isredir(c)))
            rls->token = T_WORD;
        if (rls->flag & LEX_CMDSTART
            && c == '=' && !mode && rls->word.size && !DIGIT(rls->word.data[0]))
            rls->token = T_ASSIGN_WORD;
        if (TOKEN_TYPE(rls->token) == SPECIAL|| (!mode && __ismeta(c)))
            return NO_ERROR;
    }
    else if (DFA_TERM(s))
    {
        rls->token = DFA_TOKEN(s);
        if (TOKEN_TYPE(rls->token) == KEYWORD && !(rls->flag & LEX_CMDSTART))
            rls->token = T_WORD;
    }

    vec_push(&rls->word, c);
    rls->cs->line_start = !mode && !(rls->flag & PARSER_LINE_START);

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
