#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

int CHILD_PID = -1;

void handleInput(Env* env, char* command) {
    PipeExpression* expression = GetTree(command);
    PrintPipeExpression(expression, 0);
    interpret(expression, env);
    DeletePipeExpression(expression);
}

void handleSignal(int signalNumber) {
    kill(signalNumber, CHILD_PID);
}

void interface(const int isBatch, const char** argumentsValues) {
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handleSignal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    MemContext context = MakeContext();
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);

    Env env;
    env.cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    env.childPid = &CHILD_PID;
    if (getcwd(env.cwd, COMMAND_SIZE) == NULL) {
        exit(EXIT_FAILURE);
    }

    if (isBatch) {
        FILE* input = fmemopen(argumentsValues[isBatch], strlen(argumentsValues[isBatch]), "r");
        while (fgets(command, COMMAND_SIZE, input) != NULL) {
            if (strlen(command) == 0)
                continue;
            handleInput(&env, command);
        }
    } else {
        while (strstr(command, "exit") != command) {
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);
            handleInput(&env, command);
        }
    }
    AutoExit(context);
}
