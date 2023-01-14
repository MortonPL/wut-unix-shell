#pragma once
#include <stdio.h>
#include <string.h>
#include "../lib/mmem.h"

void changeDirectory(char* cwd, const char* path);

void printWorkingDirectory(const char* cwd);

void interpret(char* cwd, char* prompt);

void interface();
