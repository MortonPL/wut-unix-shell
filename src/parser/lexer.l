/* PROLOGUE */

%{
    #include <stdio.h>
    #include "parser.h"
%}

%option noinput nounput
%option outfile="lexer.c" header-file="lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge


/* DECLARATIONS */


/* LEXICAL RULES */

%%

";"  { return OP_EXPR_END; }
"|"  { return OP_PIPE; }
"<"  { return OP_PULL; }
">"  { return OP_PUSH; }

[a-zA-z_][a-zA-Z0-9_]*"="         { yylval->text = yytext;
                                    return VARIABLE_WRITE; }
\'(.)*\'                          { yytext[strlen(yytext) - 1] = '\0';
                                    yylval->text = yytext + 1;
                                    return STRING_PART; }
\\.                               { yylval->text = yytext + 1;
                                    return STRING_PART; }
[^ \r\n\t\f\v=<>|\\'$;]+          { yylval->text = yytext;
                                    return STRING_PART; }
"="                               { yylval->text = yytext;
                                    return STRING_PART; }
"$"[a-zA-z_][a-zA-Z0-9_]*         { yylval->text = yytext + 1;
                                    return VARIABLE_READ; }
[ \r\t\f\v]+                      { return WHITESPACES; }
\n                                { continue; }
.                                 { return YYUNDEF; }

%%


/* EPILOGUE */

int yyerror(char **expression, yyscan_t scanner, const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    fprintf(stderr, "\tparsing expression: %s\n", *expression);
    (void)scanner;
    return 0;
}
