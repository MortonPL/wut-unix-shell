#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    WE_BASIC_STRING,
    WE_ESCAPED_STRING,
    WE_VARIABLE_READ,
    WE_VARIABLE_WRITE
} WordElementType;

typedef struct {
    const WordElementType Type;
    char * const Value;
    const size_t Length;
} WordElement;

typedef enum {
    CW_BASIC,
    CW_ASSIGNMENT,
    CW_REDIRECTION_IN,
    CW_REDIRECTION_OUT
} CommandWordType;

typedef struct {
    CommandWordType Type;
    WordElement **Elements;
    size_t Length;
} CommandWord;

typedef struct {
    CommandWord **Words;
    size_t Length;
    bool AnyNonAssignment;
} CommandExpression;

typedef struct PipeExpression {
    CommandExpression **Commands;
    size_t Length;
} PipeExpression;

PipeExpression *CreatePipeExpression(CommandExpression *pFirst);
CommandExpression *CreateCommandExpression(CommandWord *pFirst);
CommandWord *CreateCommandWord(WordElementType type, const char *pFirst);

bool AppendToPipeExpression(PipeExpression *pExpression, CommandExpression *pNext);
bool AppendToCommandExpression(CommandExpression *pExpression, CommandWord *pNext);
bool AppendToCommandWord(CommandWord* pWord, WordElementType typeNext, const char *pNext);

void DeletePipeExpression(PipeExpression *pExpression);
void DeleteCommandExpression(CommandExpression *pExpression);
void DeleteCommandWord(CommandWord *pWord);
void DeleteWordElement(WordElement *pElement);
