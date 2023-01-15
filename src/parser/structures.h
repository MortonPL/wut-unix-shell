#pragma once

typedef struct {
    char *Name;
    char *Value;
} AssignmentExpression;

typedef struct {
    AssignmentExpression **Assignments;
    char **Arguments;
    char *InRedir;
    char *OutRedir;
} CommandExpression;

typedef struct PipeExpression {
    struct PipeExpression *Left;
    CommandExpression *Right;
} PipeExpression;

PipeExpression *CreatePipeExpression(PipeExpression *pLeft, CommandExpression *pRight);
CommandExpression *CreateCommandExpression(AssignmentExpression **pAssignments, char **pArguments, char *pInRedir, char *pOutRedir);
AssignmentExpression *CreateAssignmentExpression(char *pName, char *pValue);

void DeletePipeExpression(PipeExpression *pExpression);
void DeleteCommandExpression(CommandExpression *pExpression);
void DeleteAssignmentExpression(AssignmentExpression *pExpression);
