#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2


typedef struct {
    char stack[COMMAND_SIZE][COMMAND_SIZE];
    int redirectIn;
    int redirectOut;
} CommandContext;

/// returns found value in key
void arrayget(char* const* env, char* key) {
    char* variable;
    size_t i = 0;
    while (*env[i] != '\0') {
        if ((variable = strstr(env[i], key)) != NULL) {
            strcpy(key, env[i] + 1 + strlen(key));
            break;
        }
    }
}

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

void doPrintWorkingDirectory(const char *file, char* const* argv, char* const* envp) {
    char cwd[COMMAND_SIZE] = "cwd";
    arrayget(envp, cwd);
    printf("%s\n", cwd);
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
    // TODO handle pipes (everywhere) and redirects (in build in)
    char* command = ctx->stack[0];
    if (strcmp(command, "") != 0) {
        if (strstr(command, "cd") == command) {
            if (argumentCount > 1) {
                changeDirectory(ctx->stack[1], env);
            }
        } else if (strstr(command, "pwd") == command) {
            int pid = attach_command(ctx->redirectIn, ctx->redirectOut, doPrintWorkingDirectory, command, ctx->stack, env->variables);
            if (pid < 0) {
                exit(pid);
            }
            *(env->childPid) = pid;
            int err = wait_for_child(pid);
            if (err < 0) {
                exit(err);
            }
            *(env->childPid) = -1;

            // printWorkingDirectory(env->cwd);
        } else if (strstr(command, "echo") == command) {
            char flags[FLAG_AMOUNT + 1] = "--\0";
            unsigned skip = 0;
            if (argumentCount > 1)
                skip = getFlags(flags, ctx);
            print(flags, skip, ctx);
        } else if (strstr(command, "exit") == command) {
            // handled in interface()
        } else {
            // TODO this forever steals stdout for some reason
            int pid = attach_command(ctx->redirectIn, ctx->redirectOut, NULL, command, ctx->stack, env->variables);
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
        arrayget(env, varName);
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
        char value[256] = {0};
        char temp[256] = {0};
        strcpy(value, element->Elements[0]->Value);
        handleWordElement(element->Elements[1], temp, env);
        strcat(value, temp);
        env->variables[env->variableCount++] = value;
        return 0;
    } else if (element->Type == CW_REDIRECTION_IN) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        ctx->redirectIn = file_in(name);
        return 0;
    } else if (element->Type == CW_REDIRECTION_OUT) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        ctx->redirectOut = file_out(name);
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
    }
    ctx.redirectIn = STDIN_FILENO;
    ctx.redirectOut = STDOUT_FILENO;
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
