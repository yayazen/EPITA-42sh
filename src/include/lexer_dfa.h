#pragma once

/*!
 * Entry state for the DFA
 */
#define DFA_ENTRY_STATE 0
/*!
 * Error state for DFA (failed transition)
 */
#define DFA_ERR_STATE -1
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
 * \brief   The token associated with the state `s`
 * \param s a state
 * \return a token or -1 if dfa_exit(`s`) is false
 * \see token.h
 */
int dfa_token(int state);

/*!
 * \brief   True if `state` is a terminal state (a token)
 * \param s a state
 * \return boolean
 */
int dfa_term(int state);

/*!
 * \brief   evaluate the DFA for the given state and symbol
 * \param c the transition character
 * \param s the current state
 * \return the new state or DFA_ERR_STATE
 */
int dfa_eval(int s, int c);
