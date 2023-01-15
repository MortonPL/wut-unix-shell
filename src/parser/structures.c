#include <stdlib.h>
#include <string.h>
#include "structures.h"

static char *copyStringWithLength(const char *pSource, size_t srcLength, size_t destLength)
{
    char *pCopy = (char *)malloc(destLength);
    if (pCopy == NULL)
        return NULL;
    memcpy(pCopy, pSource, srcLength);
    return pCopy;
}

static char *copyString(const char *pSource)
{
    size_t length = strlen(pSource) + 1;
    return copyStringWithLength(pSource, length, length);
}

static char *concatStrings(const char *pBase, const char *pNext)
{
    size_t baseLength = strlen(pBase);
    size_t nextLength = strlen(pNext);
    char *pConcat = copyStringWithLength(pBase, baseLength + 1, baseLength + nextLength + 1);
    if (pConcat == NULL)
        return NULL;
    memcpy(pConcat + baseLength, pNext, nextLength + 1);
    return pConcat;
}

static char *concatStringsWithEqBetween(const char *pBase, const char *pNext)
{
    size_t baseLength = strlen(pBase);
    size_t nextLength = strlen(pNext);
    char *pConcat = copyStringWithLength(pBase, baseLength + 1, baseLength + nextLength + 2);
    if (pConcat == NULL)
        return NULL;
    pConcat[baseLength] = '=';
    memcpy(pConcat + baseLength + 1, pNext, nextLength + 1);
    return pConcat;
}

PipeExpression *CreatePipeExpression(PipeExpression *pLeft, CommandExpression *pRight)
{
    PipeExpression *pExpression = (PipeExpression *)malloc(sizeof(PipeExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Left = pLeft;
    pExpression->Right = pRight;
    return pExpression;
}

static size_t getElementCount(CommandExpression *pExpression)
{
    size_t length = 0;
    if (pExpression == NULL)
        return length;
    CommandElement *ptr = *(pExpression->Elements);
    while (ptr++ != NULL)
        length++;
    return length;
}

CommandExpression *CreateCommandExpression(CommandElement *pFirst)
{
    CommandExpression *pExpression = (CommandExpression *)malloc(sizeof(CommandExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Elements = (CommandElement **)malloc(sizeof(CommandElement *) * 2);
    if (pExpression->Elements == NULL) {
        free(pExpression);
        return NULL;
    }

    (pExpression->Elements)[0] = pFirst;
    (pExpression->Elements)[1] = NULL;
    return pExpression;
}

static CommandElement *createCommandElement(CommandElementType type)
{
    CommandElement *pElement = (CommandElement *)malloc(sizeof(CommandElement));
    if (pElement == NULL)
        return NULL;

    CommandElement typedElement = { type, NULL, NULL };
    memcpy(pElement, &typedElement, sizeof(CommandElement));
    return pElement;
}

CommandElement *CreateAssignment(const char *pString)
{
    if (pString == NULL)
        return NULL;

    CommandElement *pElement = createCommandElement(CE_ASSIGNMENT);
    if (pElement == NULL)
        return NULL;

    char *pCopy = copyString(pString);
    if (pCopy == NULL) {
        free(pElement);
        return NULL;
    }

    pElement->Name = pCopy;
    return pElement;
}

CommandElement *CreateWord(const char *pString)
{
    if (pString == NULL)
        return NULL;

    CommandElement *pElement = createCommandElement(CE_WORD);
    if (pElement == NULL)
        return NULL;

    char *pCopy = copyString(pString);
    if (pCopy == NULL) {
        free(pElement);
        return NULL;
    }

    pElement->Value = pCopy;
    return pElement;
}

CommandElement *ConvertToRedirection(bool isOut, CommandElement *pBase)
{
    if (pBase == NULL)
        return NULL;

    if (pBase->Type == CE_REDIRECTION_OUT && isOut)
        return pBase;
    if (pBase->Type == CE_REDIRECTION_IN && !isOut)
        return pBase;

    CommandElement *pElement = createCommandElement(isOut ? CE_REDIRECTION_OUT : CE_REDIRECTION_IN);
    if (pElement == NULL)
        return NULL;

    if (pBase->Type == CE_WORD)
    {
        pElement->Name = pBase->Name;
        pElement->Value = pBase->Value;
        return pElement;
    }

    char *pConcat = concatStringsWithEqBetween(pBase->Name, pBase->Value);
    if (pConcat == NULL) {
        free(pElement);
        return NULL;
    }

    pElement->Value = pConcat;
    DeleteCommandElement(pBase);
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

    CommandElement *ptr = *(pExpression->Elements);
    while (ptr != NULL)
        free(ptr++);
    free(pExpression->Elements);

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

bool AppendToCommandElement(CommandElement *pElements, const char *pString)
{
    if (pElements == NULL || pString == NULL)
        return false;

    if (pElements->Value == NULL)
    {
        pElements->Value = copyString(pString);
        return true;
    }

    char *pOldValue = pElements->Value;
    char *pNewValue = concatStrings(pElements->Value, pString);
    if (pNewValue == NULL)
        return false;
    pElements->Value = pNewValue;
    free(pOldValue);
    return true;
}

bool AppendToCommandExpression(CommandExpression *pExpression, CommandElement *pElement)
{
    if (pExpression == NULL || pElement == NULL || pExpression->Elements == NULL)
        return false;

    size_t oldElementCount = getElementCount(pExpression);
    CommandElement **pNewElements = (CommandElement **)realloc(pExpression->Elements, sizeof(CommandElement *) * (oldElementCount + 2));
    if (pNewElements == NULL)
        return NULL;

    pExpression->Elements = pNewElements;
    (pExpression->Elements)[oldElementCount] = pElement;
    (pExpression->Elements)[oldElementCount + 1] = NULL;
    return true;
}
