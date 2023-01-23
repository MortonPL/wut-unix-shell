#include "cli.h"

#define COMMAND_SIZE 4096
#define FLAG_AMOUNT 2

void handleInput(ExecutionCtx* env, char* command) {
    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    while (expression != NULL) {
#ifdef DEBUG
        PrintPipeExpression(expression, 0);
#endif
        interpret(expression, env);
        DeletePipeExpression(expression);
        expression = ReadPipeExpression(&lexerState);
    }
}

void killCommandsSignal(int signalNumber) {
    (void)signalNumber;
    kill_commands();
}

void interface(const int isBatch, const char** argumentsValues) {
    struct sigaction sa_kill;
    sa_kill.sa_flags = SA_RESTART;
    sa_kill.sa_handler = killCommandsSignal;
    sigaction(SIGINT, &sa_kill, NULL);
    sigaction(SIGQUIT, &sa_kill, NULL);

    char command[COMMAND_SIZE];

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
            if (fgets(command, COMMAND_SIZE, stdin) != NULL)
                handleInput(&ectx, command);
        }
    }
}
