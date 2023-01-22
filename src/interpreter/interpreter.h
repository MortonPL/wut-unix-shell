#pragma once
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../pipes/pipes.h"
#include "../parser/interface.h"
#include "../parser/structures.h"
#include "../lib/mmem.h"

typedef struct {
    int next_pipe_in;
} ExecutionCtx;

typedef struct {
    char **args;
    char **eenv;
    char *redir_in;
    char *redir_out;
} CommandCtx;

// void changeDirectory(char* cwd, const char* path);

// void printWorkingDirectory(const char* cwd);

// void print(const char* prompt, const char* flags);

void interpret(PipeExpression* prompt, ExecutionCtx* env);

void kill_commands();
