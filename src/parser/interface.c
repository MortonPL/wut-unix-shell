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
