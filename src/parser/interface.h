#pragma once

#include "structures.h"

PipeExpression *GetTree(const char *pLine);

void PrintPipeExpression(PipeExpression *pExpression, int indent);
void PrintCommandExpression(CommandExpression *pExpression, int indent);
void PrintCommandElement(CommandElement *pElement, int indent);