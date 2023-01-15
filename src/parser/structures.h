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
    CommandElement *Assignments;
    CommandElement *Arguments;
    CommandElement *Redirections;
} CommandExpression;

typedef struct PipeExpression {
    struct PipeExpression *Left;  // if empty, PipeExpression only wraps CommandExpression Right
    CommandExpression *Right;
} PipeExpression;

PipeExpression *CreatePipeExpression(PipeExpression *pLeft, CommandExpression *pRight);
CommandExpression *CreateCommandExpression(CommandElement *pAssignments, CommandElement *pArguments, CommandElement *pRedirections);

CommandElement *CreateAssignment(char *pName, char *pValue);
CommandElement *CreateWord(char *pValue);
CommandElement *CreateRedirection(bool isOut, char *pValue);

void DeletePipeExpression(PipeExpression *pExpression);
void DeleteCommandExpression(CommandExpression *pExpression);

void DeleteCommandElement(CommandElement *pElement);
