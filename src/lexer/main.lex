%option nounput
%option noinput

%{
    #include "parser.tab.h"
%}

/*definitions*/

%% /*rules*/
"$"[a-zA-Z][_a-zA-Z0-9]     { return VARIABLE_READ; }
[\x20\r\t\f\v]+             { return WHITESPACES; }
[a-zA-Z][_a-zA-Z0-9]"="     { return VARIABLE_WRITE; }
[0-9a-zA-Z]                 { yylval=yytext; return STRING_PART; }
\n                          { return YYUNDEF; }

%% /*user actions*/
