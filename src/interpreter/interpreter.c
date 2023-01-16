#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

char stack[COMMAND_SIZE][COMMAND_SIZE] = {{0}};
char previousWorkingDirectory[COMMAND_SIZE] = "~";

unsigned getFlags(char* flags) {
    unsigned long currentFlag = 0;
    unsigned i = 1;
    while (*stack[i] != '\0' && *stack[i] == '-' && currentFlag < FLAG_AMOUNT) {
        char flag[COMMAND_SIZE];
        strcpy(flag, stack[i]);
        removeAllOccurences(flag, '-');
        strcpy(&flags[currentFlag], flag);
        currentFlag += strlen(flag);
        i++;
    }
    return i - 1;
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

void print(const char* flags, unsigned skip) {
    // TODO handle variables
    if (strstr(flags, "e") != NULL) {
        // TODO implement
    }
    unsigned i = 1 + skip;
    char* element = stack[i];
    while (strcmp(element, "\0") != 0) {
        printf("%s ", element);
        i++;
        element = stack[i];
    }
    if (strstr(flags, "n") == NULL) {
        printf("\n");
    }
}

void handleCommand(int argumentCount, Env* env) {
    // TODO add "export"
    char* command = stack[0];
    if (strstr(command, "cd") == command) {
        if (argumentCount > 1) {
            changeDirectory(env->cwd, stack[1]);
        }
    } else if (strstr(command, "pwd") == command) {
        printWorkingDirectory(env->cwd);
    } else if (strstr(command, "echo") == command) {
        char flags[FLAG_AMOUNT + 1] = "--\0";
        unsigned skip = 0;
        if (argumentCount > 1)
            skip = getFlags(flags);
        print(flags, skip);
    } else if (strstr(command, "exit") == command) {
        // handled in interface()
    } else {
        // TODO handle pipes and redirects
        char* path = getenv("PATH");
        char pathenv[strlen(path) + sizeof("PATH=")];
        char* envp[] = {pathenv, NULL};

        // TODO this forever steals stdout for some reason
        int pid = attach_command(STDIN_FILENO, STDOUT_FILENO, NULL, command, stack, envp);
        if (pid < 0) {
            exit(pid);
        }
        *(env->childPid) = pid;
        int err = wait_for_child(pid);
        if (err < 0) {
            exit(err);
        }
        *(env->childPid) = -1;
    }
}

// CE_ASSIGNMENT,
// CE_WORD,
// CE_REDIRECTION_IN,
// CE_REDIRECTION_OUT

void handleCommandElement(CommandElement* element, int i) {
    printf("%d\n", element->Type);
    strcpy(stack[i], element->Value);
}

void handleCommandExpression(CommandExpression* expression, Env* env) {
    if (expression->Elements == NULL) {
        exit(EXIT_FAILURE);
    } else {
        for (int a=0; a<COMMAND_SIZE; a++) {
            for (int b=0; b<COMMAND_SIZE; b++) {
                stack[a][b] = 0;
            }
        }
        CommandElement* element;
        int i = 0;
        while ((element = expression->Elements[i]) != NULL) {
            handleCommandElement(expression->Elements[i], i);
            i++;
        }
        // int size = 1; // TODO change to expressions->ElementsSize
        // for(int i = 0; i < size; i++) {
        //     handleCommandElement(expression->Elements[i]);
        // }
        handleCommand(i, env);
    }
}

void handlePipeExpression(PipeExpression* expression, Env* env) {
    if (expression->Left != NULL) {
        handlePipeExpression(expression->Left, env);
    }
    if (expression->Right == NULL) {
        exit(EXIT_FAILURE);
    } else {
        handleCommandExpression(expression->Right, env);
    }
}

void interpret(PipeExpression* prompt, Env* env) {
    handlePipeExpression(prompt, env);
}
