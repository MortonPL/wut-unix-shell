#include <stdio.h>

#include "dummy.h"

typedef int (*helloer)(const char* msg, ...);

void superprint(helloer fun)
{
    fun("Hello ");
}

int main()
{
    superprint(printf);

    printf("UXP1A.\n");
    return 0;
}
