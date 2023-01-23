#include <stdio.h>
#include "lib/logger.c"
#include "interpreter/cli.h"

int parseArgs(const int ac, const char** av, int* isBatch)
{
    int i = 1;
    while (i < ac) {
        if (strcmp(av[i], "-c") == 0) {
            if (++i < ac) {
                *isBatch = i;
                return 0;
            } else {
                fprintf(stderr, "Missing command.\n");
                return 1;
            }
        }
        i++;
    }
    return 0;
}


int main(const int argumentsCount, const char *argumentsValues[]) {
    init_logger();
    log_info("Shell started.");

    int isBatch = 0;
    if (parseArgs(argumentsCount, argumentsValues, &isBatch) != 0)
        return 1;

    interface(isBatch, argumentsValues);

    drop_logger();
    return 0;
}
