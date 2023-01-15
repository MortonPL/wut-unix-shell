#include "cli.h"

#define COMMAND_SIZE 20
#define FLAG_AMOUNT 2

char previousWorkingDirectory[COMMAND_SIZE] = "~";

void getFlags(char* flags, char** prompt) {
    unsigned long currentFlag = 0;
    while (*prompt != NULL && strstr(*prompt, "-") == *prompt && currentFlag < FLAG_AMOUNT) {
        char* flag = strsep(prompt, " ");
        removeAllOccurences(flag, '-');
        strcpy(&flags[currentFlag], flag);
        currentFlag += strlen(flag);
    }
}

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
    if (chdir(cwd) < 0) {
        exit(EXIT_FAILURE);
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
        if (prompt != NULL)
            printf("%s", prompt);
    } else {
        if (prompt != NULL)
            printf("%s\n", prompt);
        else
            printf("\n");
    }
}

void interpret(char* cwd, char* prompt) {
    // TODO add "export"
    char* command = strsep(&prompt, " ");
    if (strstr(command, "cd") == command) {
        if (prompt != NULL) {
            changeDirectory(cwd, prompt);
        }
    } else if (strstr(command, "pwd") == command) {
        printWorkingDirectory(cwd);
    } else if (strstr(command, "echo") == command) {
        // TODO handle variables
        char flags[FLAG_AMOUNT + 1] = "--\0";
        if (prompt != NULL)
            getFlags(flags, &prompt);
        print(prompt, flags);
    } else if (strstr(command, "exit") == command) {
        // handled in interface()
    } else {
        // TODO handle pipes and redirects
        char* path = getenv("PATH");
        char pathenv[strlen(path) + sizeof("PATH=")];
        char* envp[] = {pathenv, NULL};
        char* args[] = {command, prompt, NULL};

        int pid = attach_command(STDIN_FILENO, STDOUT_FILENO, NULL, command, args, envp);
        if (pid < 0) {
            exit(pid);
        }
        wait_for_child(pid);
    }
}
