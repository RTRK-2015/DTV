#ifndef COMMON_H
#define COMMON_H


// C includes
/*#include <stdlib.h>
#include <string.h>
#include <errno.h>*/


#define nameof(x) #x


#define FAIL(fmt, ...) \
    do \
    { \
        fprintf(stderr, "%s:%d:%s: Error: "fmt, \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)


#define FAIL_STD(fmt, ...) \
    FAIL("%s: "fmt, strerror(errno), ##__VA_ARGS__)


#endif

