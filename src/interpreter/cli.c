#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

int CHILD_PID = -1;

void handleInput(Env* env, char* command) {
    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    while (expression != NULL) {
        PrintPipeExpression(expression, 0);
        interpret(expression, env);
        DeletePipeExpression(expression);
        expression = ReadPipeExpression(&lexerState);
    }
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

    // init Env
    Env env;
    env.cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    env.childPid = &CHILD_PID;
    env.variableCount = 0;
    if (getcwd(env.cwd, COMMAND_SIZE) == NULL) {
        exit(EXIT_FAILURE);
    }
    strcpy(env.previousWorkingDirectory, env.cwd);

    // actual body
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
