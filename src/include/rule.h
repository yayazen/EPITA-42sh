/**
 * \file rule.h
 * \brief Parser rules function definition. This file also contains the
 * functions used to execute the AST.
 */

#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#include "token.h"

#define RL_DEFAULT_STATE                                                       \
    {                                                                          \
        .err = NO_ERROR, .flag = LEX_COLLECT | LEX_CMDSTART, .token = T_EOF,   \
        .cs = NULL, .node = NULL, .buffered_token = -1                         \
    }
/**
 * \brief anonymous enum of rule types.
 * \see rule.def
 */
enum
{
#define RULE(Rule, Str) Rule,
#include "rule.def"
#undef RULE
};

#include "rule_exec.h"

struct ctx;

/**
 * \brief represents a state in the current parsing process.
 */
struct rl_state
{
    /** \brief store error code for rule functions */
    int err;
    /** \brief flags for the parser */
    int flag;
    /** \brief the current token being processed */
    int token;
    /** \brief hold the token's string representation */
    struct vec word;
    /** \brief the stream to collect from */
    struct cstream *cs;
    /** \brief the resulting execution at this parsing state */
    struct rl_exectree *node;

    /** \brief Buffered token's word to return in priority */
    struct vec buffered_word;
    /** \brief Buffered token to return in priority */
    int buffered_token;
};

/**
 * \brief accept a token and build the corresponding ast node
 *          of rule type `rltype`.
 *  \param a rule state
 *  \param an accepted token
 */
int rl_accept(struct rl_state *s, int token);

/** \brief put current token in buffer */
void rl_buffer_token(struct rl_state *s);

/**
 * \brief same as rl_accept but errored out when token mismatch
 */
int rl_expect(struct rl_state *s, int token);

/* simple_cmd: WORD* */
int rl_simple_cmd(struct rl_state *s);
/* return is similar to execvp (must be forked) */
int rl_exec_simple_cmd(const struct ctx *ctx, struct rl_exectree *node);

/* command: fundec | simple_command | shell_cmd */
int rl_cmd(struct rl_state *s);
int rl_exec_cmd(const struct ctx *ctx, struct rl_exectree *node);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);
int rl_exec_input(const struct ctx *ctx, struct rl_exectree *s);

/* list: and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);
int rl_exec_list(const struct ctx *ctx, struct rl_exectree *s);

/* shell_cmd: if_clause */
int rl_shell_cmd(struct rl_state *s);
int rl_exec_shell_cmd(const struct ctx *ctx, struct rl_exectree *s);

/** \brief subshell: compound_list */
int rl_subshell(struct rl_state *s);
int rl_exec_subshell(const struct ctx *ctx, struct rl_exectree *node);

/* if_clause: If compound_list Then compound_list [else_clause] Fi */
int rl_if_clause(struct rl_state *s);
int rl_exec_if_clause(const struct ctx *ctx, struct rl_exectree *s);

/* else_clause:   Else compound_list
                | Elif compound_list Then compound_list [else_clause] */
int rl_else_clause(struct rl_state *s);
int rl_exec_else_clause(const struct ctx *ctx, struct rl_exectree *s);

/* rule_while: While compound_list do_group */
int rl_while(struct rl_state *s);
int rl_exec_while(const struct ctx *ctx, struct rl_exectree *s);

/* rule_until:  Until compound_list do_group */
int rl_until(struct rl_state *s);
int rl_exec_until(const struct ctx *ctx, struct rl_exectree *s);

/*  rule_for: For WORD ([';']|[('\n')* 'in' (WORD)* (';'|'\n')]) ('\n')*
 * do_group */
int rl_for(struct rl_state *s);
int rl_exec_for(const struct ctx *ctx, struct rl_exectree *s);

/* do_group: Do compound_list Done */
int rl_do_group(struct rl_state *s);
int rl_exec_do_group(const struct ctx *ctx, struct rl_exectree *s);

/* compound_list: list */
int rl_compound_list(struct rl_state *s);
int rl_exec_compound_list(const struct ctx *ctx, struct rl_exectree *s);

/* and_or: pipeline (('&&'|'||') ('\n')* pipeline)* */
int rl_and_or(struct rl_state *s);
int rl_exec_and_or(const struct ctx *ctx, struct rl_exectree *node);

/* pipeline: ['!'] command ('|' ('\n')* command)* */
int rl_pipeline(struct rl_state *s);
int rl_exec_pipeline(const struct ctx *ctx, struct rl_exectree *node);

/*   [IONUMBER] '>' WORD
 * | [IONUMBER] '<' WORD
 * | [IONUMBER] '>>' WORD
 * | [IONUMBER] '>&' WORD
 * | [IONUMBER] '<&' WORD
 * | [IONUMBER] '>|' WORD
 * | [IONUMBER] '<>' WORD
 */
int rl_redirection(struct rl_state *s);
int rl_exec_redirection(const struct ctx *ctx, struct rl_exectree *node);

/*   ASSIGNEMENT_WORD
 * | redirection
 */
int rl_prefix(struct rl_state *s);

/*   WORD
 * | redirection
 */
int rl_element(struct rl_state *s);

/* fundec: WORD '(' ')' ('\n')* shell_command */
int rl_fundec(struct rl_state *s);
int rl_exec_fundec(const struct ctx *ctx, struct rl_exectree *node);

/* rule_case: Case WORD ('\n')* 'in' ('\n')* [case_clause] Esac */
int rl_case(struct rl_state *s);
int rl_exec_case(const struct ctx *ctx, struct rl_exectree *node);

/* case_clause: case_item (';;' ('\n')* case_item)* [;;] ('\n')* */
int rl_case_clause(struct rl_state *s);
int rl_exec_case_clause(const struct ctx *ctx, struct rl_exectree *node,
                        const char *word);

/* case_item: ['('] WORD ('|' WORD)* ')' ('\n')* [ compound_list ] */
int rl_case_item(struct rl_state *s);
