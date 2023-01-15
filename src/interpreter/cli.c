#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

void interface(const int isBatch, const char** argumentsValues) {
    MemContext context = MakeContext();
    char* cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    strcpy(cwd, "~");

    // FILE* input = stdin;

    if (isBatch) {
        while (fgets(command, COMMAND_SIZE, fmemopen(argumentsValues[isBatch], strlen(argumentsValues[isBatch]), "r")) != NULL) {
            if (strlen(command) == 0)
                continue;
            PipeExpression *pResult = GetTree(command);
            DeletePipeExpression(pResult);
            printf("parsed\n");
        }
    } else {
        while (strstr(command, "exit") != command) {
            if (strlen(command) == 0)
                continue;
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);

            printf("command: %s \n", command);
            PipeExpression *expression = GetTree(command);
            DeletePipeExpression(expression);
            printf("parsed\n");

            removeAllOccurences(command, '\n');
            // TODO use parser
            interpret(cwd, command);
        }
    }
    AutoExit(context);
}
