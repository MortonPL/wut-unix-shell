#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

void interface(const int isBatch) {
    MemContext context = MakeContext();
    char* cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    strcpy(cwd, "~");

    if (isBatch) {
        // while (1) {
        //     int res = Scan();
        //     printf("%d\n", res);
        // }
    } else {
        while (strstr(command, "exit") != command) {
            if (strlen(command) == 0)
                continue;
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);
            printf("command: %s \n", command);

            const char *expression = GetTree(command);
            printf("expression: %s \n", expression);

            removeAllOccurences(command, '\n');
            // TODO use parser
            interpret(cwd, command);
        }
    }
    AutoExit(context);
}
