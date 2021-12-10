#include <assert.h>

#include "lexer.h"
#include "token.h"

/*!
 * Number of grammar symbols (Ascii Table)
 */
#define DFA_NSYM 256
/*!
 *  Upper-bound for the number of state
 */
#define DFA_NSTATE 512
/*!
 *  DFA state key index
 */
#define DFA_TOKEN DFA_NSYM

/*!
 * \brief    The DFA 2D array
 *  dfa[S][DFA_NSYM] holds the token for exit states.
 */
static int dfa[DFA_NSTATE][DFA_NSYM + 1];
static int it = -1;

/*!
 * \brief memset `n` 32bit words with n
 */
static inline void *__wmemset(void *s, int c, size_t n)
{
    int *m = s;
    for (size_t i = 0; i < n; i++)
        m[i] = c;
    return m;
}

/*!
 * \brief initialize the DFA for the lexer (Kind-Of Thread safe)
 * \see token.h
 */
static void __dfa_init(void)
{
    it = DFA_ENTRY_STATE + 1;
    __wmemset(dfa, DFA_ERR_STATE, DFA_NSTATE * (DFA_NSYM + 1));

    for (int key = 0; key < TOKEN_COUNT; key++)
    {
        int i = DFA_ENTRY_STATE;
        for (const char *p = TOKEN_STR(key); *p != '\0'; p++)
        {
            int c = *p;
            dfa[i]['\0'] = i;
            if (dfa[i][c] == DFA_ERR_STATE)
                dfa[i][c] = it++;
            i = dfa[i][c];
        }
        dfa[i]['\0'] = i;
        dfa[i][DFA_TOKEN] = key;
    }
}

int dfa_term(int state)
{
    return dfa_token(state) != DFA_ERR_STATE;
}

int dfa_token(int state)
{
    if (it == -1)
        __dfa_init();
    assert(0 <= state && state < it);
    return dfa[state][DFA_TOKEN];
}

int dfa_eval(int c, int state)
{
    if (it == -1)
        __dfa_init();
    assert(state < it && c < DFA_NSYM);
    return dfa[state][c];
}
