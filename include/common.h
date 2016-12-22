/*! \file common.h
    \brief Contains functions that all modules use.
*/
#ifndef COMMON_H
#define COMMON_H


// C includes
#include <errno.h>
#include <stdlib.h>
#include <string.h>


/// \brief Gets the string representation of its parameter.
#define nameof(x) #x


/// \brief Prints an error message to stderr and exits the program.
/// \param fmt A printf-like format string.
/// \param ... printf-like arguments to print.
#define FAIL(fmt, ...) \
    do \
    { \
        fprintf(stderr, "%s:%d:%s: Error: "fmt, \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)


/// \brief Same as \ref FAIL, except that it also prints the string
/// representation of errno.
/// \param fmt A printf-like format string.
/// \param ... printf-like arguments to print.
#define FAIL_STD(fmt, ...) \
    FAIL("%s: "fmt, strerror(errno), ##__VA_ARGS__)

/// \brief Prints out a log message.
/// \param module A string that identifies the module in which LOG is called.
/// \param fmt A printf-like format string.
/// \param ... printf-like arguments to print.
#define LOG(module, fmt, ...) \
    do \
    { \
        printf(module": "fmt, ##__VA_ARGS__); \
    } while (0)



#endif

