#include "cli.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2

void handleInput(ExecutionCtx* env, char* command) {
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

void killCommandsSignal(int signalNumber) {
    (void)signalNumber;
    kill_commands();
}

void childSignal(int signalNumber) {
    (void)signalNumber;
    printf("child process started");
}

void interface(const int isBatch, const char** argumentsValues) {
    struct sigaction sa_kill, sa_child;
    sa_kill.sa_flags = SA_RESTART;
    sa_kill.sa_handler = killCommandsSignal;
    sa_child.sa_flags = SA_RESTART;
    sa_child.sa_handler = childSignal;
    sigaction(SIGINT, &sa_kill, NULL);
    sigaction(SIGQUIT, &sa_kill, NULL);
    sigaction(SIGCHLD, &sa_child , NULL);

    MemContext context = MakeContext();
    char* command = (char*) AutoMalloc(context, COMMAND_SIZE, free);

    // init Env
    ExecutionCtx ectx = {
        .next_pipe_in = -1,
    };

    // actual body
    if (isBatch) {
        FILE* input = fmemopen((void *)(argumentsValues[isBatch]), strlen(argumentsValues[isBatch]), "r");
        while (fgets(command, COMMAND_SIZE, input) != NULL) {
            if (strlen(command) == 0)
                continue;
            handleInput(&ectx, command);
        }
    } else {
        while (strstr(command, "exit") != command) {
            printf("> ");
            fgets(command, COMMAND_SIZE, stdin);
            handleInput(&ectx, command);
        }
    }
    AutoExit(context);
}
