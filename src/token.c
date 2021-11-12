#include "token.h"

/*
 * @brief   internal token description table.
 * @comment should not be used! use TOKEN_DESC_[MEMBER] macro instead.
 */
const struct token __token_desc[TOKEN_LIST_SIZE] = {
#define TOKEN(K, S, T)                                                         \
    [K] = { .key = K, .type = T, .str = S, .len = sizeof(S) - 1 },
    TOKEN_LIST
#undef TOKEN
};
