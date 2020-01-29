#ifndef HELPER_H
#define HELPER_H

#define DEBUG

#include <stdio.h>

// debug print macro
#if defined(DEBUG)
#define DEBUG_PRINT(fmt, args...)                                              \
    fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,   \
            ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

// panic macro
#define PANIC(fmt, args...)                                                    \
    {                                                                          \
        fprintf(stderr, "ERROR: %s:%d:%s(): " fmt, __FILE__, __LINE__,         \
                __func__, ##args);                                             \
        exit(1);                                                               \
    }

// helper functions
int getAllTestCases(char name[][256]);
char* token_to_str(int token);

#endif