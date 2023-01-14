#pragma once
#include <stdio.h>
#include <string.h>
#include "../lib/strpls.h"

void changeDirectory(char* cwd, const char* path);

void printWorkingDirectory(const char* cwd);

void print(const char* prompt, const char* flags);

void interpret(char* cwd, char* prompt);
