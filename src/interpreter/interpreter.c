#include "cli.h"

#define COMMAND_SIZE 20
#define FLAG_AMOUNT 2

char previousWorkingDirectory[COMMAND_SIZE] = "~";

void changeDirectory(char* cwd, const char* path) {
    if (strstr(path, "-") == path) {
        strswp(cwd, previousWorkingDirectory);
    } else {
        strcpy(previousWorkingDirectory, cwd);
        if (strstr(path, "..") == path) {
            removeAllAfter(cwd, '/');
        } else if (strstr(path, "/") == path) {
            strcpy(cwd, path);
        } else {
            strcat(cwd, "/");
            strcat(cwd, path);
        }
    }
}

void printWorkingDirectory(const char* cwd) {
    printf("%s\n", cwd);
}

void print(const char* prompt, const char* flags) {
    // TODO handle variables
    if (strstr(flags, "e") != NULL) {
        // TODO implement
    }
    if (strstr(flags, "n") != NULL) {
        printf("%s", prompt);
    } else {
        printf("%s\n", prompt);
    }
}

void interpret(char* cwd, char* prompt) {
    // TODO add "export"
    removeAllOccurences(prompt, '\n');
    char* command = strsep(&prompt, " ");
    if (strstr(command, "cd") == command) {
        if (prompt != NULL) {
            changeDirectory(cwd, prompt);
        }
    } else if (strstr(command, "pwd") == command) {
        printWorkingDirectory(cwd);
    } else if (strstr(command, "echo") == command) {
        char flags[FLAG_AMOUNT + 1] = "--\0";
        unsigned long currentFlag = 0;
        while (prompt != NULL && strstr(prompt, "-") == prompt && currentFlag < FLAG_AMOUNT) {
            char* flag = strsep(&prompt, " ");
            removeAllOccurences(flag, '-');
            strcpy(&flags[currentFlag], flag);
            currentFlag += strlen(flag);
        }
        print(prompt, flags);
    } else if (strstr(command, "exit") == command) {
        // handled in interface()
    } else {
        char shPrompt[COMMAND_SIZE];
        shPrompt[0] = '\0';
        strcat(shPrompt, "cd ");
        strcat(shPrompt, cwd);
        strcat(shPrompt, "; ");
        strcat(shPrompt, command);
        if (prompt != NULL) {
            strcat(shPrompt, prompt);
        }
        system(shPrompt);
    }
}
