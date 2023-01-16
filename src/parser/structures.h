#pragma once

#include <stdbool.h>

typedef enum {
    CE_ASSIGNMENT,
    CE_WORD,
    CE_REDIRECTION_IN,
    CE_REDIRECTION_OUT
} CommandElementType;

typedef struct {
    const CommandElementType Type;

    char *Value;
    char *Name;  // only if Type == CE_ASSIGNMENT
} CommandElement;

typedef struct {
    CommandElement **Elements;
} CommandExpression;

typedef struct PipeExpression {
    struct PipeExpression *Left;  // if empty, PipeExpression only wraps CommandExpression Right
    CommandExpression *Right;
} PipeExpression;

PipeExpression *CreatePipeExpression(PipeExpression *pLeft, CommandExpression *pRight);
CommandExpression *CreateCommandExpression(CommandElement *pFirst);

CommandElement *CreateAssignment(const char *pFirst);  // copies pFirst
CommandElement *CreateWord(const char *pFirst);  // copies pFirst
CommandElement *ConvertToRedirection(bool isOut, CommandElement *pBase);  // frees pBase if pBase->Type == ASSIGNMENT, copies pointers otherwise

void DeletePipeExpression(PipeExpression *pExpression);
void DeleteCommandExpression(CommandExpression *pExpression);

void DeleteCommandElement(CommandElement *pElement);

bool AppendToCommandElement(CommandElement *pElements, const char *pString);  // copies pString
bool AppendToCommandExpression(CommandExpression *pExpression, CommandElement *pElement);
