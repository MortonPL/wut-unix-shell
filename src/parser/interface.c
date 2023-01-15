#include <stdio.h>
#include "structures.h"
#include "interface.h"
#include "parser.h"
#include "lexer.h"

int yyparse(PipeExpression **pExpression, yyscan_t pScanner);

void PrintTree(const char *pLine)
{
    PipeExpression *pExpression = NULL;
    yyscan_t pScanner;
    YY_BUFFER_STATE pState;

    if (yylex_init(&pScanner)) {
        /* could not initialize */
        return;
    }

    pState = yy_scan_string(pLine, pScanner);

    int result;
    for (int i = 0; !(result = yyparse(&pExpression, pScanner)); i++) {
        fprintf(stderr, "yyparse#%d returned %d\n", i, result);
        PrintPipeExpression(pExpression, 0);
        if (pExpression == NULL)
            break;
        DeletePipeExpression(pExpression);
        pExpression = NULL;
    }

    yy_delete_buffer(pState, pScanner);

    yylex_destroy(pScanner);
    
    return;
}

static void printIndents(int count)
{
    for (int i = 0; i < count; i++)
        fputs(" ", stderr);
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
    fputs("Left\n", stderr);
    PrintPipeExpression(pExpression->Left, indent + 2);
    printIndents(indent + 1);
    fputs("Right\n", stderr);
    PrintCommandExpression(pExpression->Right, indent + 2);
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
    fputs("Elements", stderr);
    if (pExpression->Elements == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    CommandElement **ptr = pExpression->Elements;
    while (*ptr != NULL)
        PrintCommandElement(*ptr++, indent + 2);
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
    fputs("CommandElement", stderr);
    if (pElement == NULL) {
        fputs(" (NULL)\n", stderr);
        return;
    }
    fputs("\n", stderr);

    printIndents(indent + 1);
    fprintf(stderr, "Type: %s\n", getCommandElementType(pElement->Type));
    printIndents(indent + 1);
    fprintf(stderr, "Name: %s\n", pElement->Name == NULL ? "NULL" : pElement->Name);
    printIndents(indent + 1);
    fprintf(stderr, "Value: %s\n", pElement->Value == NULL ? "NULL" : pElement->Value);
}
