#include "debug.h"

#include <execinfo.h>
#include <unistd.h>

void print_backtrace()
{
    void *buf[50];
    int num = backtrace(buf, sizeof(buf) / sizeof(void *));
    backtrace_symbols_fd(buf, num, STDERR_FILENO);
}

void abort_with_backtrace()
{
    char *test = NULL;
    test[0] = 'c';
}