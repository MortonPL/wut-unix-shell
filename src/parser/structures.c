#include <stdlib.h>
#include "structures.h"

PipeExpression *CreatePipeExpression(PipeExpression *pLeft, CommandExpression *pRight)
{
    PipeExpression *pExpression = (PipeExpression *)malloc(sizeof(PipeExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Left = pLeft;
    pExpression->Right = pRight;
    return pExpression;
}

CommandExpression *CreateCommandExpression(AssignmentExpression **pAssignments, char **pArguments, char *pInRedir, char *pOutRedir)
{
    CommandExpression *pExpression = (CommandExpression *)malloc(sizeof(CommandExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Assignments = pAssignments;
    pExpression->Arguments = pArguments;
    pExpression->InRedir = pInRedir;
    pExpression->OutRedir = pOutRedir;
    return pExpression;
}

AssignmentExpression *CreateAssignmentExpression(char *pName, char *pValue)
{
    AssignmentExpression *pExpression = (AssignmentExpression *)malloc(sizeof(AssignmentExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Name = pName;
    pExpression->Value = pValue;
    return pExpression;
}

void DeletePipeExpression(PipeExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    DeletePipeExpression(pExpression->Left);
    DeleteCommandExpression(pExpression->Right);

    free(pExpression);
}

static void deleteAssignmentExpressions(AssignmentExpression **pAssignments)
{
    if (pAssignments == NULL)
        return;

    AssignmentExpression *ptr = *pAssignments;
    while (ptr != NULL)
        DeleteAssignmentExpression(ptr++);
    free(pAssignments);
}

static void deleteArguments(char **pArguments)
{
    if (pArguments == NULL)
        return;

    char *ptr = *pArguments;
    while (ptr != NULL)
        free(ptr++);
    free(pArguments);
}

void DeleteCommandExpression(CommandExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    deleteAssignmentExpressions(pExpression->Assignments);
    deleteArguments(pExpression->Arguments);
    free(pExpression->InRedir);
    free(pExpression->OutRedir);

    free(pExpression);
}

void DeleteAssignmentExpression(AssignmentExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    free(pExpression->Name);
    free(pExpression->Value);

    free(pExpression);
}

