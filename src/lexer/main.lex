%option nounput
%option noinput

%{
    #include "parser.tab.h"
%}

/*variable:     [a-zA-z][a-zA-Z0-9_]*       */
/*whitespace:   [\x20\r\t\f\v]+             */

%% /*rules*/
";"                         { return OP_EXPR_END; }
"|"                         { return OP_PIPE; }
"<"                         { return OP_PULL; }
">"                         { return OP_PUSH; }
\'(.)*\'                    { yylval=yytext;    return QUOTED_STRING; }
\\.                         { yylval=++yytext;  return ESCAPED_STRING; }
[^\x20\r\t\f\v<>|\\'$;]+    { yylval=yytext;    return UNQUOTED_STRING; }
[^\x20\r\t\f\v<>|\\'$;=]+   { yylval=yytext;    return UNQUOTED_STRING_NO_EQ; }
"$"[a-zA-z][a-zA-Z0-9_]*    { yylval=++yytext;  return VARIABLE_READ; }
[a-zA-z][a-zA-Z0-9_]*"="    { yylval=yytext;    return VARIABLE_WRITE; }
[\x20\r\t\f\v]+             { return WHITESPACES; }
\n                          { return YYUNDEF; }

%% /*user actions*/
