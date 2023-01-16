#include <stdio.h>
#include "structures.h"
#include "interface.h"
#include "parser.h"
#include "lexer.h"


bool InitializeLexer(LexerState *pState, const char *pLine)
{
    if (pState == NULL || pLine == NULL)
        return false;
    if (yylex_init(&(pState->Scanner)))
        return false;
    pState->State = yy_scan_string(pLine, pState->Scanner);
    return true;
}

PipeExpression *ReadPipeExpression(LexerState *pState)
{
    if (pState == NULL)
        return NULL;

    PipeExpression *pExpression = NULL;
    int result = yyparse(&pExpression, pState->Scanner);
    if (result == 0)
        return pExpression;
    return NULL;
}

void CleanupLexer(LexerState *pState)
{
    yy_delete_buffer(pState->State, pState->Scanner);
    yylex_destroy(pState->Scanner);
}

static void printIndents(int count)
{
    for (int i = 0; i < count; i++)
        fputs(" ", stderr);
}

static const char *getCommandWordType(CommandWordType type)
{
    if (type == CW_BASIC)
        return "CW_BASIC";
    if (type == CW_ASSIGNMENT)
        return "CW_ASSIGNMENT";
    if (type == CW_REDIRECTION_IN)
        return "CW_REDIRECTION_IN";
    if (type == CW_REDIRECTION_OUT)
        return "CW_REDIRECTION_OUT";
    return "???";
}

static const char *getWordElementType(WordElementType type)
{
    if (type == WE_BASIC_STRING)
        return "WE_BASIC_STRING";
    if (type == WE_ESCAPED_STRING)
        return "WE_ESCAPED_STRING";
    if (type == WE_VARIABLE_READ)
        return "WE_VARIABLE_READ";
    if (type == WE_VARIABLE_WRITE)
        return "WE_VARIABLE_WRITE";
    return "???";
}

void PrintPipeExpression(PipeExpression *pExpression, int indent)
{
    printIndents(indent);
    fputs("PipeExpression", stderr);
    if (pExpression == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    printIndents(indent + 1);
    fprintf(stderr, "Commands[%zu]", pExpression->Length);
    if (pExpression->Commands == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    CommandExpression **ptr = pExpression->Commands;
    while (*ptr != NULL)
        PrintCommandExpression(*ptr++, indent + 2);
}

void PrintCommandExpression(CommandExpression *pExpression, int indent)
{
    printIndents(indent);
    fputs("CommandExpression", stderr);
    if (pExpression == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    printIndents(indent + 1);
    fprintf(stderr, "Words[%zu]", pExpression->Length);
    if (pExpression->Words == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    CommandWord **ptr = pExpression->Words;
    while (*ptr != NULL)
        PrintCommandWord(*ptr++, indent + 2);
}

void PrintCommandWord(CommandWord *pWord, int indent)
{
    printIndents(indent);
    fputs("CommandWord", stderr);
    if (pWord == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    printIndents(indent + 1);
    fprintf(stderr, "Type: %s\n", getCommandWordType(pWord->Type));

    printIndents(indent + 1);
    fprintf(stderr, "Elements[%zu]", pWord->Length);
    if (pWord->Elements == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    WordElement **ptr = pWord->Elements;
    while (*ptr != NULL)
        PrintWordElement(*ptr++, indent + 2);
}

void PrintWordElement(WordElement *pElement, int indent)
{
    printIndents(indent);
    fputs("WordElement", stderr);
    if (pElement == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    printIndents(indent + 1);
    fprintf(stderr, "Type: %s\n", getWordElementType(pElement->Type));
    printIndents(indent + 1);
    fprintf(stderr, "Value[%zu]: %s\n", pElement->Length, pElement->Value == NULL ? "NULL" : pElement->Value);
}
