%{  // prologue

  #include <stdio.h>

  int yylex(void);
  void yyerror(char const*);

%}


// declarations
%define api.value.type {char*}

%token STRING_PART
%token VARIABLE_READ
%token WHITESPACES
%token VARIABLE_WRITE

%left '|'
%left '<' '>'
%precedence ';'


%%  // grammar rules

expressions:
  expression expressions.trail.rep
;

expressions.trail.rep:
  %empty
| ';' expression expressions.trail.rep
;

expression:
  assignments.opt pipe_expression
;

assignments.opt:
  %empty
| assignment assignments.trail.rep
;

assignments.trail.rep:
  %empty
| assignment assignments.trail.rep
;

assignment:
  VARIABLE_WRITE string
;

pipe_expression:
  redirection_expression pipe_expression.trail.rep
;

pipe_expression.trail.rep:
  %empty
| '|' whitespaces.opt redirection_expression pipe_expression.trail.rep
;

redirection_expression:
  command redirection_expression.trail.rep
;

redirection_expression.trail.rep:
  %empty
| redirection string redirection_expression.trail.rep
;

command:
  command_string command.trail.rep
;

command.trail.rep:
  %empty
| string command.trail.rep
;

command_string:
  string
;

string:
  string_part string_part.rep
;

string_part.rep:
  whitespaces.opt
| string_part string_part.rep
;

string_part:
  STRING_PART
| VARIABLE_READ
;

redirection:
  '<' whitespaces.opt
| '>' whitespaces.opt
;

whitespaces.opt:
  %empty
| WHITESPACES
;

%%

// epilogue

int main (void)
{
  return yyparse();
}

void yyerror (char *const s)
{
  fprintf(stderr, "ERROR on line %d: %s\n", lineno);
}
