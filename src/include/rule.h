#pragma once

#include <assert.h>
#include <io/cstream.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/vec.h>

#define RL_DEFAULT_STATE                                                       \
    {                                                                          \
        .err = NO_ERROR, .flag = LEX_COLLECT | LEX_CMDSTART, .token = T_EOF,   \
        .cs = cs, .ast = NULL,                                                 \
    }

// Hack for test units, which are written in C++, which
// requires explicit cast when returning from malloc functions
#ifdef __cplusplus
#    define CAST_AST(a) (struct rl_ast *)(a)
#else
#    define CAST_AST(a) a
#endif

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

/**
 * \brief represents a rule in an AST format.
 */
struct rl_ast
{
    /* the node rule type */
    int type;
    /* store node exec pid */
    pid_t pid;
    /* file descriptors to replace STDIN and STDOUT */
    int fd[2];
    /* the effective value of the ast node */
    char *word;
    /* left-child */
    struct rl_ast *child;
    /* right-sibling */
    struct rl_ast *sibling;
};

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
    /* the resulting ast at this parsing state */
    struct rl_ast *ast;
};

/**
 * \brief create a new AST node with a predefined type.
 */
static inline struct rl_ast *rl_ast_new(int rltype)
{
    struct rl_ast *ast = CAST_AST(calloc(1, sizeof(struct rl_ast)));
    if (!ast)
        return NULL;
    ast->type = rltype;
    ast->pid = -1;
    ast->fd[0] = STDIN_FILENO;
    ast->fd[1] = STDOUT_FILENO;
    return ast;
}

/**
 * \brief free the AST recursively.
 */
static inline void rl_ast_free(struct rl_ast *ast)
{
    if (!ast)
        return;
    rl_ast_free(ast->child);
    rl_ast_free(ast->sibling);
    if (ast->word)
        free(ast->word);
    free(ast);
}

/**
 * \brief accept a token and build the corresponding ast node
 *          of rule type `rltype`.
 *  \param a rule state
 *  \param an accepted token
 *  \param the desired ruletype
 */
int rl_accept(struct rl_state *s, int token, int rltype);

/**
 * \brief same as rl_accept but errored out when token mismatch
 */
int rl_expect(struct rl_state *s, int token, int rltype);

/* simple_cmd: WORD* */
int rl_simple_cmd(struct rl_state *s);
/* return is similar to execvp (must be forked) */
int rl_exec_simple_cmd(struct rl_ast *ast);

/* command: simple_command | shell_cmd */
int rl_cmd(struct rl_state *s);
int rl_exec_cmd(struct rl_ast *ast);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);
int rl_exec_input(struct rl_ast *s);

/* list: and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);
int rl_exec_list(struct rl_ast *s);

/* shell_cmd: if_clause */
int rl_shell_cmd(struct rl_state *s);
int rl_exec_shell_cmd(struct rl_ast *s);

/* if_clause: If compound_list Then compound_list [else_clause] Fi */
int rl_if_clause(struct rl_state *s);
int rl_exec_if_clause(struct rl_ast *s);

/* else_clause:   Else compound_list
                | Elif compound_list Then compound_list [else_clause] */
int rl_else_clause(struct rl_state *s);
int rl_exec_else_clause(struct rl_ast *s);

/* rule_while: rule_while: While compound_list do_group */
int rl_while(struct rl_state *s);
int rl_exec_while(struct rl_ast *s);

/* do_group: Do compound_list Done */
int rl_do_group(struct rl_state *s);
int rl_exec_do_group(struct rl_ast *s);

/* compound_list: list */
int rl_compound_list(struct rl_state *s);
int rl_exec_compound_list(struct rl_ast *s);

/* and_or: pipeline (('&&'|'||') ('\n')* pipeline)* */
int rl_and_or(struct rl_state *s);
int rl_exec_and_or(struct rl_ast *ast);

/* pipeline: ['!'] command ('|' ('\n')* command)* */
int rl_pipeline(struct rl_state *s);
int rl_exec_pipeline(struct rl_ast *ast);

/*   [IONUMBER] '>' WORD
 * | [IONUMBER] '<' WORD
 * | [IONUMBER] '>>' WORD
 * | [IONUMBER] '>&' WORD
 * | [IONUMBER] '<&' WORD
 * | [IONUMBER] '>|' WORD
 * | [IONUMBER] '<>' WORD
 */
int rl_redirection(struct rl_state *s);

/*   ASSIGNEMENT_WORD
 * | redirection
 */
int rl_prefix(struct rl_state *s);

/*   WORD
 * | redirection
 */
int rl_element(struct rl_state *s);
