#include "cli.h"

#define COMMAND_SIZE 20
#define FLAG_AMOUNT 2

void interface(const int isBatch) {
    MemContext context = MakeContext();
    char* cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    strcpy(cwd, "~");

    if (isBatch) {
        while (1) {
            int res = Scan();
            printf("%d\n", res);
        }
    } else {
        while (strstr(command, "exit") != command) {
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);
            removeAllOccurences(command, '\n');
            // TODO use parser
            interpret(cwd, command);
        }
    }
    AutoExit(context);
}
