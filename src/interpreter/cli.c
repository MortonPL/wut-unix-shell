#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

void handleInput(char* cwd, char* command) {
    printf("command: %s \n", command);
    PipeExpression *expression = GetTree(command);
    DeletePipeExpression(expression);
    printf("parsed\n");

    removeAllOccurences(command, '\n');
    // TODO use parser
    interpret(cwd, command);
}

void interface(const int isBatch, const char** argumentsValues) {
    MemContext context = MakeContext();
    char* cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    strcpy(cwd, "~");

    if (isBatch) {
        FILE* input = fmemopen(argumentsValues[isBatch], strlen(argumentsValues[isBatch]), "r");
        while (fgets(command, COMMAND_SIZE, input) != NULL) {
            if (strlen(command) == 0)
                continue;
            handleInput(cwd,command);
        }
    } else {
        while (strstr(command, "exit") != command) {
            if (strlen(command) == 0)
                continue;
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);
            handleInput(cwd, command);
        }
    }
    AutoExit(context);
}
