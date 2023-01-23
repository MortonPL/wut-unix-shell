#pragma once

#include <stdbool.h>
#include <stdlib.h>

/// @brief Type of a primitive element (token) being a part of a word
typedef enum {
    WE_BASIC_STRING,
    WE_ESCAPED_STRING,
    WE_VARIABLE_READ,
    WE_VARIABLE_WRITE
} WordElementType;

/// @brief Primitive element (token) being a part of a word
typedef struct {
    const WordElementType Type;
    char * const Value;
    const size_t Length;
} WordElement;

/// @brief Type of a single command component: assignment, redirection or a basic word
typedef enum {
    CW_BASIC,
    CW_ASSIGNMENT,
    CW_REDIRECTION_IN,
    CW_REDIRECTION_OUT
} CommandWordType;

/// @brief A single command component: assignment, redirection or a basic word
typedef struct {
    CommandWordType Type;
    WordElement **Elements;
    size_t Length;
} CommandWord;

/// @brief Array of whitespace-separated words forming the command
typedef struct {
    CommandWord **Words;
    size_t Length;
    bool AnyNonAssignment;
} CommandExpression;

/// @brief Array of commands separated by pipe operators
typedef struct PipeExpression {
    CommandExpression **Commands;
    size_t Length;
} PipeExpression;


/// @brief Creates a PipeExpression containing a single CommandExpression.
///        BEWARE! Created object is responsible for freeing the passed CommandExpression.
/// @param pFirst The initial CommandExpression.
/// @returns Allocated PipeExpression or NULL in case of failure.
PipeExpression *CreatePipeExpression(CommandExpression *pFirst);

/// @brief Creates a CommandExpression containing a single CommandWord.
///        BEWARE! Created object is responsible for freeing the passed CommandWord.
/// @param pFirst The initial CommandWord.
/// @returns Allocated CommandExpression or NULL in case of failure.
CommandExpression *CreateCommandExpression(CommandWord *pFirst);

/// @brief Creates a CommandWord containing a single element of type type and value pFirst.
/// @param type The type of initial WordElement.
/// @param pFirst The value of initial WordElement. The string is copied. Passed instance has to be cleaned up by the caller.
/// @returns Allocated CommandWord or NULL in case of failure.
CommandWord *CreateCommandWord(WordElementType type, const char *pFirst);


/// @brief Appends a CommandExpression to PipeExpression.
///        BEWARE! PipeExpression is responsible for freeing the passed CommandExpression.
/// @param pExpression Modified PipeExpression.
/// @param pNext A CommandExpression to append to the PipeExpression.
/// @returns Boolean success indicator.
bool AppendToPipeExpression(PipeExpression *pExpression, CommandExpression *pNext);

/// @brief Appends a CommandWord to CommandExpression.
///        BEWARE! CommandExpression is responsible for freeing the passed CommandWord.
/// @param pExpression Modified CommandExpression.
/// @param pNext A CommandWord to append to the CommandExpression.
/// @returns Boolean success indicator.
bool AppendToCommandExpression(CommandExpression *pExpression, CommandWord *pNext);

/// @brief Appends a WordElement to CommandWord.
/// @param pExpression Modified CommandWord.
/// @param typeNext The type of appended WordElement.
/// @param pNext The value of appended WordElement. The string is copied. Passed instance has to be cleaned up by the caller.
/// @returns Boolean success indicator.
bool AppendToCommandWord(CommandWord* pWord, WordElementType typeNext, const char *pNext);


/// @brief Deallocates a PipeExpression, including all CommandExpressions, CommandWords and WordElements belonging to it.
/// @param pExpression PipeExpression to free.
void DeletePipeExpression(PipeExpression *pExpression);

/// @brief Deallocates a CommandExpression, including all CommandWords and WordElements belonging to it.
/// @param pExpression CommandExpression to free.
void DeleteCommandExpression(CommandExpression *pExpression);

/// @brief Deallocates a CommandWord, including all WordElements belonging to it.
/// @param pExpression CommandWord to free.
void DeleteCommandWord(CommandWord *pWord);

/// @brief Deallocates a WordElement.
/// @param pExpression WordElement to free.
void DeleteWordElement(WordElement *pElement);
