#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures.h"


static char *copyString(const char *pSource, size_t strLength)
{
    char *pCopy = (char *)malloc(strLength + 1);
    if (pCopy == NULL)
        return NULL;
    memcpy(pCopy, pSource, strLength + 1);
    return pCopy;
}

static WordElement *createWordElement(WordElementType type, const char *pValue)
{
    if (pValue == NULL)
        return NULL;

    size_t valueLength = strlen(pValue);
    char *pCopy = copyString(pValue, valueLength);
    if (pCopy == NULL)
        return NULL;

    WordElement *pElement = (WordElement *)malloc(sizeof(WordElement));
    if (pElement == NULL) {
        free(pCopy);
        return NULL;
    }

    WordElement typedElement = { type, pCopy, valueLength };
    memcpy(pElement, &typedElement, sizeof(WordElement));
    return pElement;
}


PipeExpression *CreatePipeExpression(CommandExpression *pFirst)
{
    PipeExpression *pExpression = (PipeExpression *)malloc(sizeof(PipeExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Commands = (CommandExpression **)malloc(sizeof(CommandExpression *) * 2);
    if (pExpression->Commands == NULL) {
        free(pExpression);
        return NULL;
    }

    pExpression->Length = 1;
    (pExpression->Commands)[0] = pFirst;
    (pExpression->Commands)[1] = NULL;
    return pExpression;
}

CommandExpression *CreateCommandExpression(CommandWord *pFirst)
{
    CommandExpression *pExpression = (CommandExpression *)malloc(sizeof(CommandExpression));
    if (pExpression == NULL)
        return NULL;

    pExpression->Words = (CommandWord **)malloc(sizeof(CommandWord *) * 2);
    if (pExpression->Words == NULL) {
        free(pExpression);
        return NULL;
    }

    pExpression->Length = 1;
    (pExpression->Words)[0] = pFirst;
    (pExpression->Words)[1] = NULL;
    return pExpression;
}

CommandWord *CreateCommandWord(WordElementType type, const char *pFirst)
{
    WordElement *pElement = createWordElement(type, pFirst);
    if (pElement == NULL)
        return NULL;

    CommandWord *pWord = (CommandWord *)malloc(sizeof(CommandWord));
    if (pWord == NULL) {
        DeleteWordElement(pElement);
        return NULL;
    }

    pWord->Elements = (WordElement **)malloc(sizeof(WordElement *) * 2);
    if (pWord->Elements == NULL) {
        DeleteWordElement(pElement);
        free(pWord);
        return NULL;
    }

    pWord->Type = type == WE_VARIABLE_WRITE ? CW_ASSIGNMENT : CW_BASIC;
    pWord->Length = 1;
    (pWord->Elements)[0] = pElement;
    (pWord->Elements)[1] = NULL;
    return pWord;
}


bool AppendToPipeExpression(PipeExpression *pExpression, CommandExpression *pNext)
{
    if (pExpression == NULL || pNext == NULL || pExpression->Commands == NULL)
        return false;

    CommandExpression **pNewCommands = (CommandExpression **)realloc(pExpression->Commands, sizeof(CommandExpression *) * (pExpression->Length + 2));
    if (pNewCommands == NULL)
        return false;

    pExpression->Commands = pNewCommands;
    pExpression->Length++;
    (pExpression->Commands)[pExpression->Length - 1] = pNext;
    (pExpression->Commands)[pExpression->Length] = NULL;
    return true;
}

bool AppendToCommandExpression(CommandExpression *pExpression, CommandWord *pNext)
{
    if (pExpression == NULL || pNext == NULL || pExpression->Words == NULL)
        return false;

    CommandWord **pNewWords = (CommandWord **)realloc(pExpression->Words, sizeof(CommandWord *) * (pExpression->Length + 2));
    if (pNewWords == NULL)
        return false;

    pExpression->Words = pNewWords;
    pExpression->Length++;
    (pExpression->Words)[pExpression->Length - 1] = pNext;
    (pExpression->Words)[pExpression->Length] = NULL;
    return true;
}

bool AppendToCommandWord(CommandWord* pWord, WordElementType typeNext, const char *pNext)
{
    if (pWord == NULL || pNext == NULL || pWord->Elements == NULL)
        return false;

    WordElement *pElement = createWordElement(typeNext, pNext);
    if (pElement == NULL)
        return false;

    WordElement **pNewElements = (WordElement **)realloc(pWord->Elements, sizeof(WordElement *) * (pWord->Length + 2));
    if (pNewElements == NULL) {
        DeleteWordElement(pElement);
        return false;
    }

    pWord->Elements = pNewElements;
    pWord->Length++;
    (pWord->Elements)[pWord->Length - 1] = pElement;
    (pWord->Elements)[pWord->Length] = NULL;
    return true;
}


void DeletePipeExpression(PipeExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    CommandExpression **ptr = pExpression->Commands;
    while (*ptr != NULL)
        DeleteCommandExpression(*ptr++);
    free(pExpression->Commands);

    free(pExpression);
}

void DeleteCommandExpression(CommandExpression *pExpression)
{
    if (pExpression == NULL)
        return;

    CommandWord **ptr = pExpression->Words;
    while (*ptr != NULL)
        DeleteCommandWord(*ptr++);
    free(pExpression->Words);

    free(pExpression);
}

void DeleteCommandWord(CommandWord *pWord)
{
    if (pWord == NULL)
        return;

    WordElement **ptr = pWord->Elements;
    while (*ptr != NULL)
        DeleteWordElement(*ptr++);
    free(pWord->Elements);

    free(pWord);
}

void DeleteWordElement(WordElement *pElement)
{
    if (pElement == NULL)
        return;

    free(pElement->Value);

    free(pElement);
}
