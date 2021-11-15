#include <config.h>
#include <stdio.h>

#include "token.h"

int main(void)
{
    puts("Package : " PACKAGE_STRING ".");

#ifdef DEBUG
    int c = 0;
    printf("DEBUG << %ld tokens\n", TOKEN_COUNT);
#    define TOKEN(K, S, T) printf("[%2d] %s -> %s (%s)\n", c++, #    K, #    S, #    T);
#    include "token.def"
#    undef TOKEN
#endif

    return 0;
}
