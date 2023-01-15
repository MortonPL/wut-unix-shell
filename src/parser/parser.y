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
%lex-param   { yyscan_t scanner }
%parse-param { PipeExpression **pExpression }
%parse-param { yyscan_t scanner }

%union {
    int value;
    char *text;
    PipeExpression *pipe;
    CommandExpression *command;
    CommandElement *element;
}


/* DECLARATIONS */

%token<text> STRING_PART
%token<text> VARIABLE_READ
%token<text> VARIABLE_WRITE
%token       WHITESPACES

%left       OP_PIPE
%precedence OP_PULL OP_PUSH
/* %precedence OP_EXPR_END */

%type<pipe>    pipe_expression
%type<command> command
%type<element> argument_or_redirection
%type<element> redirection
%type<element> string


/* GRAMMAR RULES */

%%

/* expressions:
  pipe_expression
| expressions OP_EXPR_END whitespaces.opt pipe_expression.opt
; */

pipe_expression:
  command whitespaces.opt
    { $$ = CreatePipeExpression(NULL, $1); }
| pipe_expression OP_PIPE whitespaces.opt command
    { $$ = CreatePipeExpression($1, $4); }
;

pipe_expression.opt:
  %empty
| pipe_expression
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
    { $$ = ConvertToRedirection(false, $3); }
| OP_PUSH whitespaces.opt string
    { $$ = ConvertToRedirection(true, $3); }
;

string:
  STRING_PART
    { $$ = CreateWord($1); }
| VARIABLE_READ
    { $$ = CreateWord($1); }
| VARIABLE_WRITE
    { $$ = CreateAssignment($1); }
| string STRING_PART
    { $$ = $1; AppendToCommandElement($$, $2); }
| string VARIABLE_READ
    { $$ = $1; AppendToCommandElement($$, $2); }
| string VARIABLE_WRITE
    { $$ = $1; AppendToCommandElement($$, $2); }
;

whitespaces.opt:
  %empty
| WHITESPACES
;

%%


/* EPILOGUE */
