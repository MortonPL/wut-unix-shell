#include "mmem.h"

typedef struct MemData
{
    struct MemData* pNext;
    void* pData;
    size_t life;
    destructor pDestructor;
    char selfManagedData;
} MemData;

typedef struct realMemContext
{
    struct MemData* pHead;
} realMemContext;

enum 
{
    useMalloc = 0,
    useCalloc = 1,
};

enum
{
    notSelfManaged = 0,
    selfManaged = 1,
};

realMemContext globalMemContext = {.pHead = NULL};
realMemContext contextsContext = {.pHead = NULL};
MemContext GlobalMemContext = &globalMemContext;
MemContext ContextsContext = &contextsContext;

// in 2nd param: 0 for malloc, 1 for calloc
void* autoAlloc(realMemContext* pContext, size_t size, destructor pDestructor, char mallocOrCalloc, char selfManagedData)
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
    pNewHead->selfManagedData = selfManagedData;
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
    return autoAlloc(pContext, size, pDestructor, useMalloc, notSelfManaged);
}

void* AutoCalloc(realMemContext* pContext, size_t size, destructor pDestructor)
{
    return autoAlloc(pContext, size, pDestructor, useCalloc, notSelfManaged);
}

long* AutoInsert(realMemContext* pContext, long data, destructor pDestructor)
{
    void* ptr = autoAlloc(pContext, sizeof(data), pDestructor, useMalloc, selfManaged);
    if (ptr == NULL)
        return NULL;
    *(long*)ptr = data;
    return (long*)ptr;
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
            if (pNode->selfManagedData)
                free(pNode->pData);
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
