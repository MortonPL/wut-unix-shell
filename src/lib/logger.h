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

int init_logger();

/// @brief Internal function for leaving application after fatal exception
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @param fmt Format schema
/// @param ... Format args
void __panic(const char *file, int line, const char *fmt, ...);

/// @brief Internal function for leaving application in case of error with custom message
/// @param status_code Status code to check
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @param fmt Format schema
/// @param ... Format args
/// @returns Status code if non negative
int __expect(int status_code, const char *file, int line, const char *fmt, ...);

/// @brief Internal function for leaving application in case of error with strerror message
/// @param status_code Status code to check
/// @param file Source file name from macro
/// @param line Source line name from macro
/// @returns Status code if non negative
int __unwrap(int status_code, const char *file, int line);

#ifndef panic

/// @brief Macro for leaving application after fatal exception
/// @param ... Same as printf args
#define panic(...) (__panic(__FILE__, __LINE__, __VA_ARGS__))

/// @brief Macro for leaving application in case of error with custom message
/// @param status_code Status code to check
/// @param ... Same as printf args
/// @returns Status code if non negative
#define expect(status_code, ...) (__expect(status_code, __FILE__, __LINE__, __VA_ARGS__))

/// @brief Macro for leaving application in case of error with strerror message
/// @param status_code Status code to check
/// @returns Status code if non negative
#define unwrap(status_code) (__unwrap(status_code, __FILE__, __LINE__))

#endif
