#pragma once
#include <string.h>

/// @brief Swaps content of two strings.
/// @param first First string to swap.
/// @param second Second string to swap.
void strswp(char* first, char* second);

/// @brief Removes all characters after the "searched" character
/// @param destination String to remove from.
/// @param searched Character to be searched for.
void removeAllAfter(char* destination, const char searched);

/// @brief Removes all occurences of the searched character
/// @param destination String to remove from.
/// @param searched Character to be removed.
void removeAllOccurences(char* destination, const char searched);
