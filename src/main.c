#include <config.h>
#include <stdio.h>

#include "token.h"

int main(void)
{
    puts("Package : " PACKAGE_STRING ".");

#ifdef DEBUG
    int c = 0;
#define TOKEN(k, s, t) \
    printf("[%2d] %s -> %s (%s)\n", c++, #k, #s, #t);
    TOKEN_LIST
#undef TOKEN
#endif
    
    return 0;
}
