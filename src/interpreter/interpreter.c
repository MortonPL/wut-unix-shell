#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2


typedef struct {
    char stack[COMMAND_SIZE][COMMAND_SIZE];
    char redirectIn[COMMAND_SIZE];
    char redirectOut[COMMAND_SIZE];
} CommandContext;

unsigned getFlags(char* flags, CommandContext* ctx) {
    unsigned long currentFlag = 0;
    unsigned i = 1;
    while (*(ctx->stack[i]) != '\0' && *(ctx->stack[i]) == '-' && currentFlag < FLAG_AMOUNT) {
        char flag[COMMAND_SIZE];
        strcpy(flag, ctx->stack[i]);
        removeAllOccurences(flag, '-');
        strcpy(&flags[currentFlag], flag);
        currentFlag += strlen(flag);
        i++;
    }
    return i - 1;
}

void changeDirectory(const char* path, Env* env) {
    if (strstr(path, "-") == path) {
        strswp(env->cwd, env->previousWorkingDirectory);
    } else {
        strcpy(env->previousWorkingDirectory, env->cwd);
        if (strstr(path, "..") == path) {
            removeAllAfter(env->cwd, '/');
        } else if (strstr(path, "/") == path) {
            strcpy(env->cwd, path);
        } else {
            strcat(env->cwd, "/");
            strcat(env->cwd, path);
        }
    }
    if (chdir(env->cwd) < 0) {
        exit(EXIT_FAILURE);
    }
}

void printWorkingDirectory(const char* cwd) {
    printf("%s\n", cwd);
}

void print(const char* flags, unsigned skip, CommandContext* ctx) {
    if (strstr(flags, "e") != NULL) {
        // TODO implement
    }
    unsigned i = 1 + skip;
    char* element = ctx->stack[i];
    while (strcmp(element, "\0") != 0) {
        printf("%s ", element);
        i++;
        element = ctx->stack[i];
    }
    if (strstr(flags, "n") == NULL) {
        printf("\n");
    }
}

void handleCommand(int argumentCount, CommandContext* ctx, Env* env) {
    // TODO add "export"
    // TODO handle pipes and redirects
    char* command = ctx->stack[0];
    if (strcmp(command, "") != 0) {
        if (strstr(command, "cd") == command) {
            if (argumentCount > 1) {
                changeDirectory(ctx->stack[1], env);
            }
        } else if (strstr(command, "pwd") == command) {
            printWorkingDirectory(env->cwd);
        } else if (strstr(command, "echo") == command) {
            char flags[FLAG_AMOUNT + 1] = "--\0";
            unsigned skip = 0;
            if (argumentCount > 1)
                skip = getFlags(flags, ctx);
            print(flags, skip, ctx);
        } else if (strstr(command, "exit") == command) {
            // handled in interface()
        } else {
            char* path = getenv("PATH");
            char pathenv[strlen(path) + sizeof("PATH=")];
            char* envp[] = {pathenv, NULL};

            // TODO this forever steals stdout for some reason
            int pid = attach_command(STDIN_FILENO, STDOUT_FILENO, NULL, command, ctx->stack, envp);
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
}

void mapget(Env* env, char* key) {
    for (int i=0; i<env->variableCount; i++) {
        if (strcmp(env->variables[i].key, key) == 0) {
            strcpy(key, env->variables[i].value);
            break;
        }
    }
}

size_t handleWordElement(WordElement* element, char* buffer, Env* env) {
    if (element->Type == WE_BASIC_STRING) {
        strcpy(buffer, element->Value);
        return strlen(element->Value);
    } else if (element->Type == WE_ESCAPED_STRING) {
        // TODO actually escape
        strcpy(buffer, element->Value);
        return strlen(element->Value);
    } else if (element->Type == WE_VARIABLE_READ) {
        char varName[COMMAND_SIZE];
        strcpy(varName, element->Value);
        removeAllOccurences(varName, '$');
        mapget(env, varName);
        strcpy(buffer, varName);
        return strlen(varName);
    }
    return 0;
}

void handleWordElements(WordElement** elements, size_t length, char* buffer, Env* env) {
    size_t offset = 0;
    for (size_t i=0; i<length; i++) {
        offset += handleWordElement(elements[i], buffer+offset, env);
    }
}

int handleCommandWord(CommandWord* element, int stackIterator, CommandContext* ctx, Env* env) {
    if (element->Type == CW_ASSIGNMENT) {
        if (!(element->Length == 2 && element->Elements[0]->Type == WE_VARIABLE_WRITE)) {
            exit(EXIT_FAILURE);
        }
        MapEntry entry;
        for (int b=0; b<COMMAND_SIZE; b++) {
            entry.key[b] = 0;
            entry.value[b] = 0;
        }
        strcpy(entry.key, element->Elements[0]->Value);
        removeAllOccurences(entry.key, '=');
        handleWordElement(element->Elements[1], entry.value, env);
        env->variables[env->variableCount++] = entry;
        return 0;
    } else if (element->Type == CW_REDIRECTION_IN) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        strcpy(ctx->redirectIn, name);
        return 0;
    } else if (element->Type == CW_REDIRECTION_OUT) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        strcpy(ctx->redirectOut, name);
        return 0;
    } else {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        strcpy(ctx->stack[stackIterator], name);
        return 1;
    }
}

void handleCommandExpression(CommandExpression* expression, Env* env) {
    // init context
    CommandContext ctx;
    for (int a=0; a<COMMAND_SIZE; a++) {
        for (int b=0; b<COMMAND_SIZE; b++) {
            ctx.stack[a][b] = 0;
        }
        ctx.redirectIn[a] = 0;
        ctx.redirectOut[a] = 0;
    }
    // actual body
    int argumentCount = 0;
    for(size_t i = 0; i < expression->Length; i++) {
        argumentCount += handleCommandWord(expression->Words[i], argumentCount, &ctx, env);
    }
    handleCommand(argumentCount, &ctx, env);
}

void handlePipeExpression(PipeExpression* expression, Env* env) {
    for (size_t i=0; i < expression->Length; i++) {
        handleCommandExpression(expression->Commands[i], env);
    }
    // TODO actually pipe them
}

void interpret(PipeExpression* prompt, Env* env) {
    handlePipeExpression(prompt, env);
}
