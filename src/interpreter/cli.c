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

void handleSignal(int signalNumber) {
    (void)signalNumber;
    exit(-1);
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
