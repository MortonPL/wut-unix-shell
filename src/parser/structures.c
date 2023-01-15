#include <stdlib.h>
#include <string.h>
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

CommandExpression *CreateCommandExpression(CommandElement *pAssignments, CommandElement *pArguments, CommandElement *pRedirections)
{
    CommandExpression *pExpression = (CommandExpression *)malloc(sizeof(CommandExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Assignments = pAssignments;
    pExpression->Arguments = pArguments;
    pExpression->Redirections = pRedirections;
    return pExpression;
}

static CommandElement *createCommandElement(CommandElementType type)
{
    CommandElement *pElement = (CommandElement *)malloc(sizeof(CommandElement));
    if (pElement == NULL)
        return NULL;

    CommandElement typedElement = { type, NULL, NULL };
    memcpy_s(pElement, &typedElement, sizeof(CommandElement));
    return pElement;
}

CommandElement *CreateAssignment(char *pName, char *pValue)
{
    CommandElement *pElement = createCommandElement(CE_ASSIGNMENT);
    if (pElement == NULL)
        return NULL;

    pElement->Value = pValue;
    pElement->Name = pName;
    return pElement;
}

CommandElement *CreateWord(char *pValue)
{
    CommandElement *pElement = createCommandElement(CE_WORD);
    if (pElement == NULL)
        return NULL;

    pElement->Value = pValue;
    return pElement;
}

CommandElement *CreateRedirection(bool isOut, char *pValue)
{
    CommandElement *pElement = createCommandElement(isOut ? CE_REDIRECTION_OUT : CE_REDIRECTION_IN);
    if (pElement == NULL)
        return NULL;

    pElement->Value = pValue;
    return pElement;
}

void DeletePipeExpression(PipeExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    DeletePipeExpression(pExpression->Left);
    DeleteCommandExpression(pExpression->Right);

    free(pExpression);
}

void DeleteCommandExpression(CommandExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    DeleteCommandElement(pExpression->Assignments);
    DeleteCommandElement(pExpression->Arguments);
    DeleteCommandElement(pExpression->Redirections);

    free(pExpression);
}

void DeleteCommandElement(CommandElement *pElement)
{
    if (pElement == NULL)
        return;

    free(pElement->Name);
    free(pElement->Value);

    free(pElement);
}

