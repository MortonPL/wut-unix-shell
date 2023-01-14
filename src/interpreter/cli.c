#include "cli.h"

#define COMMAND_SIZE 20

void changeDirectory(char* cwd, const char* path) {
    // TODO handle ., .. and -
    if (strstr(path, "/") == path) {
        strcpy(cwd, path);
    } else {
        strcat(cwd, "/");
        strcat(cwd, path);
    }
}

void printWorkingDirectory(const char* cwd) {
    printf("%s\n", cwd);
}

void interpret(char* cwd, char* prompt) {
    // TODO echo, export
    if (strstr(prompt, "cd") == prompt) {
        char* found = strsep(&prompt, " ");
        if ((found = strsep(&prompt, " ")) != NULL) {
            changeDirectory(cwd, found);
        }
    } else if (strstr(prompt, "pwd") == prompt) {
        printWorkingDirectory(cwd);
    } else if (strstr(prompt, "exit") == prompt) {
        // handled in interface()
    } else {
        printf("Unrecognized command: %s\n", prompt);
    }
}

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
