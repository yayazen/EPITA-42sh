#include "stdio.h"

// #define LOG_DEBUG 1

#ifdef LOG_DEBUG
#    define LOG(msg)                                                           \
        {                                                                      \
            fprintf(stderr, msg);                                              \
        }
#else
#    define LOG(msg) ;
#endif

void print_backtrace();

/**
 * \brief Abort execution of program, printing a backtrace
 *
 * Yanis will kill me when he will see this function
 */
void abort_with_backtrace();