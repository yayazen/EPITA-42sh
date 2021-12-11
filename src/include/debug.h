#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>

// #define LOG_DEBUG 1

#ifdef LOG_DEBUG
#    define LOG(msg)                                                           \
        {                                                                      \
            fprintf(stderr, msg);                                              \
        }
#else
#    define LOG(msg) ;
#endif

/**
 * \brief Print current backtrace
 */
static inline void print_backtrace(void)
{
    void *buf[50];
    int num = backtrace(buf, sizeof(buf) / sizeof(void *));
    backtrace_symbols_fd(buf, num, STDERR_FILENO);
}

/**
 * \brief Abort execution of program, printing a backtrace
 *
 * Please use this function only for debugging purposes!
 *
 * Yanis will kill me when he will see this function
 */
static inline void abort_with_backtrace(void)
{
    char *test = NULL;
    test[0] = 'c';
}