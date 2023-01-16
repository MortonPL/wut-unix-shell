/* PROLOGUE */

%{
  #include "structures.h"
  #include "parser.h"
  #include "lexer.h"

  // reference the implementation provided in lexer.l
  int yyerror(PipeExpression **pExpression, yyscan_t pScanner, const char *pMessage);
%}

%code requires {
  typedef void* yyscan_t;
}

%output  "parser.c"
%defines "parser.h"

%define      api.pure
%define      parse.error verbose
%lex-param   { yyscan_t scanner }
%parse-param { PipeExpression **pExpression }
%parse-param { yyscan_t scanner }

%union {
    int value;
    char *text;
    PipeExpression *pipe;
    CommandExpression *command;
    CommandWord *word;
}


/* DECLARATIONS */

%token<text> STRING_PART
%token<text> ESCAPED_STRING_PART
%token<text> VARIABLE_READ
%token<text> VARIABLE_WRITE
%token       WHITESPACES

%left       OP_PIPE
%precedence OP_PULL OP_PUSH
%precedence OP_EXPR_END

%type<pipe>    pipe_expression
%type<command> command
%type<word>    argument_or_redirection
%type<word>    redirection
%type<word>    string


/* GRAMMAR RULES */

%%

top_expression:
  whitespaces.opt pipe_expression top_expression.trail.opt
    { *pExpression = $2; YYACCEPT; }
;

top_expression.trail.opt:
  %empty
|  OP_EXPR_END
;

pipe_expression:
  command whitespaces.opt 
    { $$ = CreatePipeExpression($1); }
| pipe_expression OP_PIPE whitespaces.opt command whitespaces.opt 
    { $$ = $1; AppendToPipeExpression($$, $4); }
;

command:
  argument_or_redirection
    { $$ = CreateCommandExpression($1); }
| command WHITESPACES argument_or_redirection
    { $$ = $1; AppendToCommandExpression($$, $3); }
| command redirection
    { $$ = $1; AppendToCommandExpression($$, $2); }
;

argument_or_redirection:
  string
| redirection
;

redirection:
  OP_PULL whitespaces.opt string
    { $$ = $3; $$->Type = CW_REDIRECTION_IN; }
| OP_PUSH whitespaces.opt string
    { $$ = $3; $$->Type = CW_REDIRECTION_OUT; }
;

string:
  STRING_PART
    { $$ = CreateCommandWord(WE_BASIC_STRING, $1); }
| ESCAPED_STRING_PART
    { $$ = CreateCommandWord(WE_ESCAPED_STRING, $1); }
| VARIABLE_READ
    { $$ = CreateCommandWord(WE_VARIABLE_READ, $1); }
| VARIABLE_WRITE
    { $$ = CreateCommandWord(WE_VARIABLE_WRITE, $1); }
| string STRING_PART
    { $$ = $1; AppendToCommandWord($$, WE_BASIC_STRING, $2); }
| string ESCAPED_STRING_PART
    { $$ = $1; AppendToCommandWord($$, WE_ESCAPED_STRING, $2); }
| string VARIABLE_READ
    { $$ = $1; AppendToCommandWord($$, WE_VARIABLE_READ, $2); }
| string VARIABLE_WRITE
    { $$ = $1; AppendToCommandWord($$, WE_VARIABLE_WRITE, $2); }
;

whitespaces.opt:
  %empty
| WHITESPACES
;

%%


/* EPILOGUE */
