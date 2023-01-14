#include "mmem.h"

typedef struct MemData
{
    struct MemData* pNext;
    void* pData;
    size_t life;
    destructor pDestructor;
} MemData;

typedef struct realMemContext
{
    struct MemData* pHead;
} realMemContext;


realMemContext globalMemContext = {.pHead = NULL};
realMemContext contextsContext = {.pHead = NULL};
MemContext GlobalMemContext = &globalMemContext;
MemContext ContextsContext = &contextsContext;

// in 2nd param: 0 for malloc, 1 for calloc
void* autoAlloc(realMemContext* pContext, size_t size, destructor pDestructor, char mallocOrCalloc)
{
    if (pContext == NULL)
        return NULL;

    void* pData;
    if (!mallocOrCalloc)
        pData = malloc(size);
    else
        pData = calloc(size, 1);
    if (pData == NULL)
        return NULL;

    MemData* pNewHead = malloc(sizeof(MemData));
    if (pNewHead == NULL)
    {
        pDestructor(pData);
        return NULL;
    }
    pNewHead->pNext = pContext->pHead;
    pNewHead->pData = pData;
    pNewHead->life = 1;
    pNewHead->pDestructor = pDestructor;
    pContext->pHead = pNewHead;
    return pNewHead->pData;
}

void memContextDestructor(void* pContext)
{
    MemData* pNode = ((realMemContext*)pContext)->pHead;
    MemData* pNext;
    while (pNode)
    {
        pNext = pNode->pNext;
        pNode->pDestructor(pNode);
        free(pNode);
        pNode = pNext;
    }
    free(pContext);
}

void* AutoMalloc(realMemContext* pContext, size_t size, destructor pDestructor)
{
    return autoAlloc(pContext, size, pDestructor, 0);
}

void* AutoCalloc(realMemContext* pContext, size_t size, destructor pDestructor)
{
    return autoAlloc(pContext, size, pDestructor, 1);
}

realMemContext* MakeContext()
{
    realMemContext* pContext;
    if ((pContext = (realMemContext*)AutoMalloc(ContextsContext, sizeof(realMemContext), memContextDestructor)) == NULL)
        return NULL;
    pContext->pHead = NULL;
    return pContext;
}

void AutoEntry(realMemContext* pContext)
{
    if (pContext == NULL)
        return;

    struct MemData* pNode = pContext->pHead;
    while (pNode)
    {
        pNode->life++;
        pNode = pNode->pNext;
    }

    if (pContext != ContextsContext)
        AutoEntry(ContextsContext);
}

void AutoExit(realMemContext* pContext)
{
    if (pContext == NULL)
        return;

    MemData* pNode = pContext->pHead;
    MemData* pPrev = NULL;
    MemData* pNext = NULL;
    while (pNode)
    {
        if(--(pNode->life) == 0)
        {
            if (pPrev)
                pPrev->pNext = pNode->pNext;
            else
                pContext->pHead = pNode->pNext;

            pNext = pNode->pNext;
            pNode->pDestructor(pNode->pData);
            free(pNode);
        }
        else
        {
            pPrev = pNode;
            pNext = pNode->pNext;
        }
        pNode = pNext;
    }

    if (pContext != ContextsContext)
        AutoExit(ContextsContext);
}

void KeepAlive(realMemContext* pContext, void* pData, size_t life)
{
    if (pContext == NULL)
        return;

    MemData* pNode = pContext->pHead;
    while (pNode)
    {
        if (pNode->pData == pData)
        {
            pNode->life += life;
            return;
        }
        pNode = pNode->pNext;
    }
}
