#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

#include "token.h"

#define RL_DEFAULT_STATE                                                       \
    {                                                                          \
        .err = NO_ERROR, .flag = LEX_COLLECT | LEX_CMDSTART, .token = T_EOF,   \
        .cs = cs, .node = NULL,                                                \
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
    /* store error code for rule functions */
    int err;
    /* flags for the parser */
    int flag;
    /* the current token being processed */
    int token;
    /* hold the token's string representation */
    struct vec word;
    /* the stream to collect from */
    struct cstream *cs;
    /* the resulting execution at this parsing state */
    struct rl_exectree *node;
};

/**
 * \brief accept a token and build the corresponding ast node
 *          of rule type `rltype`.
 *  \param a rule state
 *  \param an accepted token
 */
int rl_accept(struct rl_state *s, int token);

/**
 * \brief same as rl_accept but errored out when token mismatch
 */
int rl_expect(struct rl_state *s, int token);

/* simple_cmd: WORD* */
int rl_simple_cmd(struct rl_state *s);
/* return is similar to execvp (must be forked) */
int rl_exec_simple_cmd(struct rl_exectree *node, const struct ctx *ctx);

/* command: simple_command | shell_cmd */
int rl_cmd(struct rl_state *s);
int rl_exec_cmd(struct rl_exectree *node, const struct ctx *ctx);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);
int rl_exec_input(struct rl_exectree *s, const struct ctx *ctx);

/* list: and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);
int rl_exec_list(struct rl_exectree *s, const struct ctx *ctx);

/* shell_cmd: if_clause */
int rl_shell_cmd(struct rl_state *s);
int rl_exec_shell_cmd(struct rl_exectree *s, const struct ctx *ctx);

/* if_clause: If compound_list Then compound_list [else_clause] Fi */
int rl_if_clause(struct rl_state *s);
int rl_exec_if_clause(struct rl_exectree *s, const struct ctx *ctx);

/* else_clause:   Else compound_list
                | Elif compound_list Then compound_list [else_clause] */
int rl_else_clause(struct rl_state *s);
int rl_exec_else_clause(struct rl_exectree *s, const struct ctx *ctx);

/* rule_while: While compound_list do_group */
int rl_while(struct rl_state *s);
int rl_exec_while(struct rl_exectree *s, const struct ctx *ctx);

/* rule_until:  Until compound_list do_group */
int rl_until(struct rl_state *s);
int rl_exec_until(struct rl_exectree *s, const struct ctx *ctx);

/* do_group: Do compound_list Done */
int rl_do_group(struct rl_state *s);
int rl_exec_do_group(struct rl_exectree *s, const struct ctx *ctx);

/* compound_list: list */
int rl_compound_list(struct rl_state *s);
int rl_exec_compound_list(struct rl_exectree *s, const struct ctx *ctx);

/* and_or: pipeline (('&&'|'||') ('\n')* pipeline)* */
int rl_and_or(struct rl_state *s);
int rl_exec_and_or(struct rl_exectree *node, const struct ctx *ctx);

/* pipeline: ['!'] command ('|' ('\n')* command)* */
int rl_pipeline(struct rl_state *s);
int rl_exec_pipeline(struct rl_exectree *node, const struct ctx *ctx);

/*   [IONUMBER] '>' WORD
 * | [IONUMBER] '<' WORD
 * | [IONUMBER] '>>' WORD
 * | [IONUMBER] '>&' WORD
 * | [IONUMBER] '<&' WORD
 * | [IONUMBER] '>|' WORD
 * | [IONUMBER] '<>' WORD
 */
int rl_redirection(struct rl_state *s);
int rl_exec_redirection(struct rl_exectree *node);

/*   ASSIGNEMENT_WORD
 * | redirection
 */
int rl_prefix(struct rl_state *s);

/*   WORD
 * | redirection
 */
int rl_element(struct rl_state *s);
