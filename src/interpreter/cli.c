#include "cli.h"

#define COMMAND_SIZE 20
#define FLAG_AMOUNT 2

void interface() {
    MemContext context = MakeContext();
    char* cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    strcpy(cwd, "~");

    while (strstr(command, "exit") != command) {
        printf("> ");
        fgets(command, COMMAND_SIZE, stdin);
        interpret(cwd, command);
    }
    AutoExit(context);
}
