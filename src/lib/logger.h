#pragma once
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "log.c/src/log.h"

/// @brief Initialize logger at the beginning of app
void init_logger();

/// @brief Cleanup logger after app finished
void drop_logger();

/// @brief Log error message
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @param fmt Format schema
/// @param ... Format args
void __panic(const char *file, int line, const char *fmt, ...);

/// @brief Log error message on negative status code
/// @param status_code Status code to check
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @param fmt Format schema
/// @param ... Format args
/// @returns Status code if non negative
int __logerr(int status_code, const char *file, int line, const char *fmt, ...);

/// @brief Log system error message on negative status code
/// @param status_code Status code to check
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @returns Status code if non negative
int __logoserr(int status_code, const char *file, int line);

/// @brief Log alloc error message on null pointer
/// @param pointer Pointer
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @param ... Format args
void *__notnull(void *pointer, const char *file, int line);

extern const char *OsErrorMessage;

#ifndef panic

/// @brief Log error message and return
/// @param ... Same as printf args
#define panic(...)                                \
    do {                                          \
        __panic(__FILE__, __LINE__, __VA_ARGS__); \
        return -1;                                \
    } while(0)

/// @brief Log error message on negative status code
/// @param status_code Status code to check
/// @param ... Same as printf args
/// @returns Status code if non negative
#define logerr(status_code, ...) (__logerr(status_code, __FILE__, __LINE__, __VA_ARGS__))

/// @brief Log system error message on negative status code
/// @param status_code Status code to check
/// @returns Status code if non negative
#define logoserr(status_code) (__logoserr(status_code, __FILE__, __LINE__))

/// @brief Log alloc error message on null pointer
/// @param pointer Pointer
/// @returns Pointer if not null
#define notnull(pointer, ...) (__notnull(status_code, __FILE__, __LINE__))

/// @brief Return on negative status code
#define errreturn(status_code) \
    do {                       \
        if (status_code < 0)   \
            return -1;         \
    } while(0)

/// @brief Return on null pointer
#define nullreturn(pointer)  \
    do {                     \
        if (pointer == NULL) \
            return NULL;     \
    } while(0)

#endif
