#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

enum
{
#define RULE(Rule, Str) Rule,
#include "rule.def"
#undef RULE
};

struct rl_ast
{
    int type;
    char *word;
    struct rl_ast *child;
    struct rl_ast *sibling;
};

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

struct rl_state
{
    int err;
    int flag;
    int token;
    struct vec word;
    struct cstream *cs;
    struct rl_ast *ast;
};

int rl_accept(struct rl_state *s, int token, int rl_type);

int rl_expect(struct rl_state *s, int token, int rl_type);

/* WORD* */
int rl_simple_cmd(struct rl_state *s);
int rl_exec_simple_cmd(struct rl_ast *ast);

/* command: simple_command */
int rl_cmd(struct rl_state *s);
int rl_exec_cmd(struct rl_ast *ast);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);
int rl_exec_input(struct rl_ast *s);

/* and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);
int rl_exec_list(struct rl_ast *s);

/* pipeline (('&&'|'||') ('\n')* pipeline)* */
int rl_and_or(struct rl_state *s);

/* ['!'] command ('|' ('\n')* command)* */
int rl_pipeline(struct rl_state *s);

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
