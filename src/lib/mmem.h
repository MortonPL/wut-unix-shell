#pragma once
#include <stdlib.h>

typedef struct realMemContext* MemContext;
typedef void (*destructor)(void*);

/// @brief A globally accessible MemContext. Note that it is NOT implicitly updated when chaning scopes.
MemContext GlobalMemContext;

/// @brief Creates a new context to use locally. Context will be destroyed automatically on AutoExit when it exits its scope.
/// @return New MemContext or NULL on failure.
MemContext MakeContext();

/// @brief Allocates new momory with `malloc()` of given size and returns a pointer.
/// @param context Memory context.
/// @param size Size (in bytes) of memory to allocate.
/// @return Pointer to allocated memory or NULL on failure.
void* AutoMalloc(MemContext context, size_t size, destructor pDestructor);

/// @brief Allocates new momory with `calloc()` of given size and returns a pointer.
/// @param context Memory context.
/// @param size Size (in bytes) of memory to allocate.
/// @return Pointer to allocated memory or NULL on failure.
void* AutoCalloc(MemContext context, size_t size, destructor pDestructor);

/// @brief Extends lifetime of all objects in a context by one scope. You SHOULD call it when entering a new scope.
/// @param context Memory context.
void AutoEntry(MemContext context);

/// @brief Shortens lifetime of all objects in a context by one scope. Dead objects are automatically freed. You SHOULD call it on every exit point of a scope.
/// @param context Memory context.
void AutoExit(MemContext context);

/// @brief Arbitrarily extends lifetime of a data.
/// @param context Memory context.
/// @param pData Pointer to targeted allocated memory.
/// @param life Lifetime (measured in scopes) to add.
void KeepAlive(MemContext context, void* pData, size_t life);
