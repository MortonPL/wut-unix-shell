#pragma once
#include <stdio.h>
#include <string.h>
#include "../lib/mmem.h"
#include "../parser/interface.h"
#include "interpreter.h"

void interface(const int isBatch, const char** argumentsValues);
