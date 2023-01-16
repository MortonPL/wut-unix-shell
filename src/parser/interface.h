#pragma once

#include <stdbool.h>
#include "structures.h"
#include "parser.h"
#include "lexer.h"


typedef struct {
    yyscan_t Scanner;
    YY_BUFFER_STATE State;
} LexerState;


bool InitializeLexer(LexerState *pState, const char *pLine);
PipeExpression *ReadPipeExpression(LexerState *pState);
void CleanupLexer(LexerState *pState);

void PrintPipeExpression(PipeExpression *pExpression, int indent);
void PrintCommandExpression(CommandExpression *pExpression, int indent);
void PrintCommandWord(CommandWord *pWord, int indent);
void PrintWordElement(WordElement *pElement, int indent);
