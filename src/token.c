#include "token.h"

#include <assert.h>

static inline __check_token(register int key)
{
    return 0 <= key && key < TOKEN_COUNT;
}

int _token_get_type(register int key)
{
    static const int assoc[] = {
#define TOKEN(Key, Str, Type) [Key] = Type,
#include "token.def"
#undef TOKEN
    };
    __check_token(key);
    return assoc[key];
}

const char *_token_get_str(register int key)
{
    static const char *assoc[] = {
#define TOKEN(Key, Str, Type) [Key] = Str,
#include "token.def"
#undef TOKEN
    };
    __check_token(key);
    return assoc[key];
}

size_t _token_get_strlen(register int key)
{
    static const size_t assoc[] = {
#define TOKEN(Key, Str, Type) [Key] = sizeof(Str) - 1,
#include "token.def"
#undef TOKEN
    };
    __check_token(key);
    return assoc[key];
}
