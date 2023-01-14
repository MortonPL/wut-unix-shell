/* PROLOGUE */

%{
  #include <stdio.h>

  int yylex(void);
  void yyerror(char const *s);
%}


/* DECLARATIONS */

%define api.value.type {char*}

%token STRING_PART
%token VARIABLE_READ
%token WHITESPACES
%token VARIABLE_WRITE

%left OP_PIPE
%precedence OP_PULL OP_PUSH
%precedence OP_EXPR_END


/* GRAMMAR RULES */

%%

expressions:
  expression expressions.trail.rep
;

expressions.trail.rep:
  %empty
| OP_EXPR_END expression expressions.trail.rep
;

expression:
  pipe_expression
;

pipe_expression:
  command pipe_expression.trail.rep
;

pipe_expression.trail.rep:
  %empty
| OP_PIPE whitespaces.opt command pipe_expression.trail.rep
;

command:
  assignment assignments_or_arguments.trail.rep
| argument assignments_or_arguments.trail.rep
;

assignments_or_arguments.trail.rep:
  %empty
| WHITESPACES assignments_or_arguments.opt
| redirection assignments_or_arguments.trail.rep
;

assignments_or_arguments.opt:
  %empty
| assignment_or_argument assignments_or_arguments.trail.rep
;

assignment_or_argument:
  assignment
| argument_or_redirection
;

assignment:
  VARIABLE_WRITE string
;

argument_or_redirection:
  argument
| redirection
;

argument:
  string
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
;

whitespaces.opt:
  %empty
| WHITESPACES
;

%%


/* EPILOGUE */

int bisonMain(void)
{
  return yyparse();
}

void yyerror(char const *s)
{
  fprintf(stderr, "ERROR: %s\n", s);
}
