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

#define LEX_WORD_BREAK                                                         \
    ((!mode || (mode == LEX_MODE_DOLLAR && c != '(' && c != ')'))              \
     && TOKEN_DELIM(c))

/**
 * \brief  move `stream` to the next non-blank character
 * \param  cs a cstream
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
 * \param c the character processed by the lexer
 * \param mode the current mode for the lexer
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

    else if (c == '$' && !(mode & (LEX_MODE_SQUOTE | LEX_MODE_DOLLAR)))
        return mode | LEX_MODE_DOLLAR;

    return mode & ~LEX_MODE_DOLLAR;
}

/**
 * \brief  Evaluate a T_WORD to the appropriate DEFAULT TOKEN
 * \param rls The current parsing state
 */
static inline void __evalword(struct rl_state *rls)
{
    int c;
    if (rls->token != T_WORD || cstream_peek(rls->cs, &c) != NO_ERROR)
        return;

    if (rls->word.size == 1 && DIGIT(rls->word.data[0])
        && (c == '<' || c == '>'))
        rls->token = T_IONUMBER;
    else if (rls->flag & LEX_CMDSTART && strchr(vec_cstring(&rls->word), '='))
        rls->token = T_ASSIGN_WORD;
}

/**
 * \brief Collect a token recursively w.r.t the mode
 */
static int __lexaux(struct rl_state *rls, int mode)
{
    int state = DFA_ENTRY_STATE;
    int c;
    while (rls->token != T_LF
           && (rls->err = cstream_peek(rls->cs, &c)) == NO_ERROR && c != EOF)
    {
        state = DFA(c, state);

        if (state == DFA_ERR_STATE || rls->token == T_WORD)
        {
            if (rls->token == T_EOF)
                rls->token = T_WORD;

            if (TOKEN_TYPE(rls->token) == SPECIAL || LEX_WORD_BREAK)
                break;

            rls->token = T_WORD;
            state = DFA_ENTRY_STATE;
        }
        else if (DFA_TERM(state))
        {
            rls->token = DFA_TOKEN(state);

            if (TOKEN_TYPE(rls->token) == KEYWORD
                && !(rls->flag & LEX_CMDSTART))
                rls->token = T_WORD;
        }

        vec_push(&rls->word, c);
        rls->cs->line_start = !mode && !(rls->flag & PARSER_LINE_START);
        if ((rls->err = cstream_pop(rls->cs, &c)) != NO_ERROR)
            break;

        int sub = __lexmode(c, mode);
        if (sub != mode && (mode ^ sub) != LEX_MODE_DOLLAR)
        {
            if (!sub || __lexaux(rls, (mode ^ sub) & ~LEX_MODE_DOLLAR))
                break;
        }

        if (sub & LEX_MODE_DOLLAR)
            mode |= LEX_MODE_DOLLAR;
        else
            mode &= ~LEX_MODE_DOLLAR;
    }

    if (c == EOF)
    {
        c = cstream_pop(rls->cs, &c);
        if (rls->token == T_EOF && rls->word.size > 0)
            rls->token = T_WORD;
        if (mode & ~LEX_MODE_DOLLAR)
        {
            rls->err = LEXER_ERROR;
            rls->cs->line_start = true;
        }
    }

    return rls->err;
}

/**
 * \brief Read a new token from the stream `rls->cs` a store it in `rls->token`
 * \param rls the current parsing state
 */
int lexer(struct rl_state *rls)
{
    __eatwhitespaces(rls->cs);
    vec_reset(&rls->word);
    rls->token = T_EOF;
    __lexaux(rls, 0);
    if (rls->err == NO_ERROR && rls->token == T_WORD)
        __evalword(rls);
    return rls->err;
}
