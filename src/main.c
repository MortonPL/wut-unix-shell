#include <stdio.h>
#include <stdlib.h>
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

int main()
{
    AutoEntry(GlobalMemContext);
    superprint(printf);

    interface();

    AutoExit(GlobalMemContext);
    return 0;
}
