#pragma once

#include <assert.h>
#include <io/cstream.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/vec.h>

#define RL_DEFAULT_STATE                                                       \
    {                                                                          \
        .err = NO_ERROR, .flag = LEX_COLLECT | LEX_CMDSTART, .token = T_EOF,   \
        .cs = cs, .node = NULL,                                                 \
    }

// Hack for test units, which are written in C++, which
// requires explicit cast when returning from malloc functions
#ifdef __cplusplus
#    define CAST_AST(a) (struct rl_exectree *)(a)
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

struct exec_cmd
{
    /* store node exec pid */
    pid_t pid;
    /* file descriptors to replace STDIN, STDOUT and STDERR */
    int fd[3];
    /* execution arguments capacity */
    int arg_capacity;
    /* number of execution arguments */
    int argc;
    /* execution arguments */
    char **argv;
};


struct exec_pipeline
{
    int negate;
    int pipe[2];
};

struct exec_redir
{
    /* redirection token */ 
    int token;
    /* left side ionumber */
    int ionumber;
    /* right side filename or ionumber */
    char *filename;
};

struct exec_word
{
    char *word;
};

union exec_type
{
    struct exec_word word;
    struct exec_cmd cmd;
    struct exec_pipeline pipeline; 
    struct exec_redir redir;
};

/**
 * \brief represents a rule in an execution tree format.
 */
struct rl_exectree
{
    /* the node rule type */
    int type;
    /* left-child */
    struct rl_exectree *child;
    /* right-sibling */
    struct rl_exectree *sibling;
    /* */
    union exec_type data;

    /* TODO rm */ 
    char *word; 
    /* store node exec pid */
    pid_t pid;
    /* file descriptors to replace STDIN, STDOUT and STDERR */
    int fd[3];
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
    struct rl_exectree *node;
};

/**
 * \brief create a new AST node with a predefined type.
 */
static inline struct rl_exectree *rl_exectree_new(int rltype)
{
    struct rl_exectree *node = CAST_AST(calloc(1, sizeof(struct rl_exectree)));
    if (!node)
        return NULL;
    

    node->type = rltype;
    node->pid = -1;
    node->fd[0] = STDIN_FILENO;
    node->fd[1] = STDOUT_FILENO;
    return node;
}

/**
 * \brief free the AST recursively.
 */
static inline void rl_exectree_free(struct rl_exectree *node)
{
    if (!node)
        return;
    rl_exectree_free(node->child);
    rl_exectree_free(node->sibling);
    if (node->word)
        free(node->word);
    free(node);
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
int rl_exec_simple_cmd(struct rl_exectree *node);

/* command: simple_command | shell_cmd */
int rl_cmd(struct rl_state *s);
int rl_exec_cmd(struct rl_exectree *node);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);
int rl_exec_input(struct rl_exectree *s);

/* list: and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);
int rl_exec_list(struct rl_exectree *s);

/* shell_cmd: if_clause */
int rl_shell_cmd(struct rl_state *s);
int rl_exec_shell_cmd(struct rl_exectree *s);

/* if_clause: If compound_list Then compound_list [else_clause] Fi */
int rl_if_clause(struct rl_state *s);
int rl_exec_if_clause(struct rl_exectree *s);

/* else_clause:   Else compound_list
                | Elif compound_list Then compound_list [else_clause] */
int rl_else_clause(struct rl_state *s);
int rl_exec_else_clause(struct rl_exectree *s);

/* rule_while: While compound_list do_group */
int rl_while(struct rl_state *s);
int rl_exec_while(struct rl_exectree *s);

/* rule_until:  Until compound_list do_group */
int rl_until(struct rl_state *s);
int rl_exec_until(struct rl_exectree *s);

/* do_group: Do compound_list Done */
int rl_do_group(struct rl_state *s);
int rl_exec_do_group(struct rl_exectree *s);

/* compound_list: list */
int rl_compound_list(struct rl_state *s);
int rl_exec_compound_list(struct rl_exectree *s);

/* and_or: pipeline (('&&'|'||') ('\n')* pipeline)* */
int rl_and_or(struct rl_state *s);
int rl_exec_and_or(struct rl_exectree *node);

/* pipeline: ['!'] command ('|' ('\n')* command)* */
int rl_pipeline(struct rl_state *s);
int rl_exec_pipeline(struct rl_exectree *node);

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
