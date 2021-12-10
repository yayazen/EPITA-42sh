#pragma once

#include "rule.h"

/*!
 * Entry state for the DFA
 */
#define DFA_ENTRY_STATE 0
/*!
 * Error state for DFA (failed transition)
 */
#define DFA_ERR_STATE -1

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

/*!
 * \brief lexer entry function
 * \param a 'rule state' as defined in rule.h
 * \return NO_ERROR or an error code as defined in utils/error.h
 * \see token.h
 */
int lexer(struct rl_state *rls);
