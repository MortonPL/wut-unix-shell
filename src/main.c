#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/mmem.h"
#include "interpreter/cli.h"

typedef int (*helloer)(const char* msg, ...);

void superprint(helloer fun)
{
    // stupid little demo of stupid little automatic memory management
    // also a stupid little demo of function pointers, they might be useful (TKOM vibes)
    MemContext ctx = MakeContext();
    char* x = (char*)AutoMalloc(ctx, 6, free);
    x[0] = 'U';
    x[1] = 'X';
    x[2] = 'P';
    x[3] = '1';
    x[4] = 'A';
    x[5] = '\0';
    AutoMalloc(ctx, 100, free);

    fun("Hello %s!\n", x);
    AutoExit(ctx);
    // won't leak!
}

int main(const int argumentsCount, char *argumentsValues[])
{
    AutoEntry(GlobalMemContext);
    superprint(printf);

    int isBatch = 0;
    if (argumentsCount > 1) {
        if (strcmp(argumentsValues[1], "-c") == 0) {
            isBatch = 1;
        }
    }
    interface(isBatch);

    AutoExit(GlobalMemContext);
    return 0;
}
