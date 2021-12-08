#include "parser.h"

#include <io/cstream.h>
#include <utils/vec.h>

#include "rule.h"

int cs_parse(struct cstream *cs, int flag)
{
    struct rl_state s = { .cs = cs,
                          .flag = flag,
                          .token = -KEYBOARD_INTERRUPT };

    vec_init(&s.word);
    int rc = rl_input(&s);
    vec_destroy(&s.word);

    return rc;
}
