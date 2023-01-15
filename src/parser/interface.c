#include <stdio.h>
#include "structures.h"
#include "interface.h"
#include "parser.h"
#include "lexer.h"

int yyparse(PipeExpression **pExpression, yyscan_t pScanner);

PipeExpression *GetTree(const char *pLine)
{
    PipeExpression *pExpression = NULL;
    yyscan_t pScanner;
    YY_BUFFER_STATE pState;

    if (yylex_init(&pScanner)) {
        /* could not initialize */
        return NULL;
    }

    pState = yy_scan_string(pLine, pScanner);

    if (yyparse(&pExpression, pScanner)) {
        /* error parsing */
        return NULL;
    }

    yy_delete_buffer(pState, pScanner);

    yylex_destroy(pScanner);

    return pExpression;
}

static void printIndents(int count)
{
    for (int i = 0; i < count; i++)
        fputs(" ", stdout);
}

void PrintPipeExpression(PipeExpression *pExpression, int indent)
{
    printIndents(indent);
    fputs("PipeExpression", stdout);
    if (pExpression == NULL) {
        fputs(" (NULL)\n", stdout);
        return;
    }
    fputs("\n", stdout);

    printIndents(indent + 1);
    fputs("Left\n", stdout);
    PrintPipeExpression(pExpression->Left, indent + 2);
    printIndents(indent + 1);
    fputs("Right\n", stdout);
    PrintCommandExpression(pExpression->Right, indent + 2);
}

void PrintCommandExpression(CommandExpression *pExpression, int indent)
{
    printIndents(indent);
    fputs("CommandExpression", stdout);
    if (pExpression == NULL) {
        fputs(" (NULL)\n", stdout);
        return;
    }
    fputs("\n", stdout);

    printIndents(indent + 1);
    fputs("Elements", stdout);
    if (pExpression->Elements == NULL) {
        fputs(" (NULL)\n", stdout);
        return;
    }
    fputs("\n", stdout);

    CommandElement *ptr = *(pExpression->Elements);
    while (ptr != NULL)
        PrintCommandElement(ptr++, indent + 2);
}

static const char *getCommandElementType(CommandElementType type)
{
    if (type == CE_ASSIGNMENT)
        return "CE_ASSIGNMENT";
    if (type == CE_WORD)
        return "CE_WORD";
    if (type == CE_REDIRECTION_IN)
        return "CE_REDIRECTION_IN";
    if (type == CE_REDIRECTION_OUT)
        return "CE_REDIRECTION_OUT";
        return "???";
}

void PrintCommandElement(CommandElement *pElement, int indent)
{
    printIndents(indent);
    fputs("CommandElement", stdout);
    if (pElement == NULL) {
        fputs(" (NULL)\n", stdout);
        return;
    }
    fputs("\n", stdout);

    printIndents(indent + 1);
    printf("Type: %s\n", getCommandElementType(pElement->Type));
    printIndents(indent + 1);
    printf("Name: %s\n", pElement->Name == NULL ? "NULL" : pElement->Name);
    printIndents(indent + 1);
    printf("Value: %s\n", pElement->Value == NULL ? "NULL" : pElement->Value);
}
