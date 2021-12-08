#pragma once

#include <io/cstream.h>
#include <utils/vec.h>

struct rl_state
{
    int flag;
    int token;
    struct vec word;
    struct cstream *cs;
};

int rl_accept(struct rl_state *s, int token);

int rl_expect(struct rl_state *s, int token);

/*   list
 * | list EOF
 * | '\n'
 * | EOF
 */
int rl_input(struct rl_state *s);

/* and_or ((';'|'&') and_or)* [';'|'&'] */
int rl_list(struct rl_state *s);

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
