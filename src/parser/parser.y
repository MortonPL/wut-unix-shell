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
%precedence OP_EXPR_END

/* %type<pipe>    pipe_expression
%type<command> command
%type<element> argument_or_redirection
%type<text>    string */


/* GRAMMAR RULES */

%%

expressions:
  pipe_expression
| expressions OP_EXPR_END whitespaces.opt pipe_expression.opt
;

pipe_expression:
  command whitespaces.opt
| pipe_expression OP_PIPE whitespaces.opt command
;

pipe_expression.opt:
  %empty
| pipe_expression
;

command:
  argument_or_redirection
| command WHITESPACES argument_or_redirection
| command redirection
;

argument_or_redirection:
  string
| redirection
;

redirection:
  OP_PULL whitespaces.opt string
| OP_PUSH whitespaces.opt string
;

string:
  string_part string_part.rep
;

string_part.rep:
  %empty
| string_part string_part.rep
;

string_part:
  STRING_PART
| VARIABLE_READ
| VARIABLE_WRITE
;

whitespaces.opt:
  %empty
| WHITESPACES
;

%%


/* EPILOGUE */
