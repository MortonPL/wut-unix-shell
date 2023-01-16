#pragma once

#include "structures.h"

void PrintTree(const char *pLine);

void PrintPipeExpression(PipeExpression *pExpression, int indent);
void PrintCommandExpression(CommandExpression *pExpression, int indent);
void PrintCommandWord(CommandWord *pWord, int indent);
void PrintWordElement(WordElement *pElement, int indent);
