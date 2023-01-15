/* PROLOGUE */

%option nounput
%option noinput

%{
    #include "parser.tab.h"
%}

/* DECLARATIONS */

/* LEXICAL RULES */

%%
    /*variable:     [a-zA-z][a-zA-Z0-9_]*       */
    /*whitespace:   [\x20\r\t\f\v]+             */

";"                         {
                                return OP_EXPR_END;
                            }
"|"                         {
                                return OP_PIPE;
                            }
"<"                         {
                                return OP_PULL;
                            }
">"                         {
                                return OP_PUSH;
                            }
\'(.)*\'                    {
                                yytext[strlen(yytext)-1] = '\0';
                                yylval=yytext+1;
                                return STRING_PART;
                            }
\\.                         {
                                yylval=yytext+1;
                                return STRING_PART;
                            }
[^ \r\n\t\f\v<>|\\'$;]+     {
                                yylval=yytext;
                                return STRING_PART;
                            }
"$"[a-zA-z][a-zA-Z0-9_]*    {
                                yylval=yytext+1;
                                return VARIABLE_READ;
                            }
[a-zA-z][a-zA-Z0-9_]*"="    {
                                yytext[strlen(yytext)-1] = '\0';
                                yylval=yytext;
                                return VARIABLE_WRITE;
                            }
[ \r\t\f\v]+                {
                                return WHITESPACES;
                            }
\n                          {
                                /* ignore EOL? */
                            }
.                           {
                                return YYUNDEF;
                            }

%%

/* EPILOGUE */
