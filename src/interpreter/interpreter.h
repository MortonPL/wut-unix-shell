#pragma once
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../lib/strpls.h"
#include "../pipes/pipes.h"
#include "../parser/interface.h"
#include "../parser/structures.h"
#include "../lib/mmem.h"

typedef struct {
    char* cwd;
    char previousWorkingDirectory[256];
    int* childPid;
    char* variables[256];
    int variableCount;
} Env;

// void changeDirectory(char* cwd, const char* path);

// void printWorkingDirectory(const char* cwd);

// void print(const char* prompt, const char* flags);

void interpret(PipeExpression* prompt, Env* env);
