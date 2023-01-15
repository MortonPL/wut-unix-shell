#include <stdio.h>
#include "tree.h"
#include "parser.h"
#include "lexer.h"

int yyparse(const char **expression, yyscan_t scanner);

const char *GetTree(const char *line)
{
    const char *expression;
    yyscan_t scanner;
    YY_BUFFER_STATE state;

    if (yylex_init(&scanner)) {
        /* could not initialize */
        return NULL;
    }

    state = yy_scan_string(line, scanner);

    if (yyparse(&expression, scanner)) {
        /* error parsing */
        return NULL;
    }

    yy_delete_buffer(state, scanner);

    yylex_destroy(scanner);

    return expression;
}
