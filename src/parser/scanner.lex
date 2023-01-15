/* PROLOGUE */

%option nounput
%option noinput

%{
    #include "parser.tab.h"
%}


/* DECLARATIONS */


/* LEXICAL RULES */

%%

";"  { return OP_EXPR_END; }
"|"  { return OP_PIPE; }
"<"  { return OP_PULL; }
">"  { return OP_PUSH; }

[a-zA-z_][a-zA-Z0-9_]*"="         { yylval = yytext;
                                    return VARIABLE_WRITE; }
\'(.)*\'                          { yytext[strlen(yytext) - 1] = '\0';
                                    yylval = yytext + 1;
                                    return STRING_PART; }
\\.                               { yylval = yytext + 1;
                                    return STRING_PART; }
[^ \r\n\t\f\v=<>|\\'$;]+          { yylval = yytext;
                                    return STRING_PART; }
"="                               { yylval = yytext;
                                    return STRING_PART; }
"$"[a-zA-z_][a-zA-Z0-9_]*         { yylval = yytext + 1;
                                    return VARIABLE_READ; }
[ \r\t\f\v]+                      { return WHITESPACES; }
\n                                { continue; }
.                                 { return YYUNDEF; }

%%


/* EPILOGUE */
