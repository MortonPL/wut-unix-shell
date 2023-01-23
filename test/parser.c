#include <tau/tau.h>
#include "../parser/interface.h"

// Don't know how to use this?
// See: https://github.com/jasmcaus/tau/blob/dev/docs/tau-primer.md
// Or: ./tau/docs/tau-primer.md


void CHECK_PIPE_EXPR(PipeExpression* expression, size_t length) {
    CHECK_NOT_NULL(expression);
    CHECK_EQ(expression->Length, length);
    CHECK_NOT_NULL(expression->Commands);
    for (size_t i=0; i<length; i++) {
        CHECK_NOT_NULL(expression->Commands[i]);
    }
}

void CHECK_COMM_EXPR(CommandExpression* expression, size_t length) {
    CHECK_NOT_NULL(expression);
    CHECK_EQ(expression->Length, length);
    CHECK_NOT_NULL(expression->Words);
    for (size_t i=0; i<length; i++) {
        CHECK_NOT_NULL(expression->Words[i]);
    }
}

void CHECK_COMM_WORD(CommandWord* expression, CommandWordType type, size_t length) {
    CHECK_NOT_NULL(expression);
    CHECK_EQ(expression->Type, type);
    CHECK_EQ(expression->Length, length);
    CHECK_NOT_NULL(expression->Elements);
}

void CHECK_WORD_ELEM(WordElement* elem, char* value, WordElementType type, size_t length) {
    CHECK_NOT_NULL(elem);
    CHECK_STREQ(elem->Value, value);
    CHECK_EQ(elem->Type, type);
    CHECK_EQ(elem->Length, length);
}

TEST(GrammarTestSuite, pwdTest) {
    char* command = "pwd";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 1);
    CHECK_EQ(expression->Commands[0]->Words[0]->Type, CW_BASIC);
    CHECK_EQ(expression->Commands[0]->Words[0]->Length, 1);
    CHECK_NOT_NULL(expression->Commands[0]->Words[0]->Elements);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "pwd", WE_BASIC_STRING, 3);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, cdFooTest) {
    char* command = "cd foo";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "cd", WE_BASIC_STRING, 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "foo", WE_BASIC_STRING, 3);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, echo123Test) {
    char* command = "echo 1 2 3";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 4);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "echo", WE_BASIC_STRING, 4);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "1", WE_BASIC_STRING, 1);
    CHECK_COMM_WORD(expression->Commands[0]->Words[2], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[2]->Elements[0], "2", WE_BASIC_STRING, 1);
    CHECK_COMM_WORD(expression->Commands[0]->Words[3], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[3]->Elements[0], "3", WE_BASIC_STRING, 1);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, redirectTest) {
    char* command = "cat < a.txt";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_REDIRECTION_IN, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "a.txt", WE_BASIC_STRING, 5);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, redirect2Test) {
    char* command = "cat <a.txt >b.txt";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 3);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_REDIRECTION_IN, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "a.txt", WE_BASIC_STRING, 5);
    CHECK_COMM_WORD(expression->Commands[0]->Words[2], CW_REDIRECTION_OUT, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[2]->Elements[0], "b.txt", WE_BASIC_STRING, 5);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, pipeTest) {
    char* command = "pwd | cat|cat | cat";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 4);
    CHECK_COMM_EXPR(expression->Commands[0], 1);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "pwd", WE_BASIC_STRING, 3);
    CHECK_COMM_EXPR(expression->Commands[1], 1);
    CHECK_COMM_WORD(expression->Commands[1]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[1]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    CHECK_COMM_EXPR(expression->Commands[2], 1);
    CHECK_COMM_WORD(expression->Commands[2]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[2]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    CHECK_COMM_EXPR(expression->Commands[3], 1);
    CHECK_COMM_WORD(expression->Commands[3]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[3]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, pipeAndRedirectTest) {
    char* command = "pwd | cat<b.txt";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 2);
    CHECK_COMM_EXPR(expression->Commands[0], 1);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "pwd", WE_BASIC_STRING, 3);
    CHECK_COMM_EXPR(expression->Commands[1], 2);
    CHECK_COMM_WORD(expression->Commands[1]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[1]->Words[0]->Elements[0], "cat", WE_BASIC_STRING, 3);
    CHECK_COMM_WORD(expression->Commands[1]->Words[1], CW_REDIRECTION_IN, 1);
    CHECK_WORD_ELEM(expression->Commands[1]->Words[1]->Elements[0], "b.txt", WE_BASIC_STRING, 5);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, quoteTest) {
    char* command = "cd 'a 'b' c'";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "cd", WE_BASIC_STRING, 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "a 'b' c", WE_ESCAPED_STRING, 7);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, quote2Test) {
    char* command = "'ech'o 1";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "ech", WE_ESCAPED_STRING, 3);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[1], "o", WE_BASIC_STRING, 1);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "1", WE_BASIC_STRING, 1);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, assignmentTest) {
    char* command = "a1=11 a2=12 c==";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 3);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_ASSIGNMENT, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "a1=", WE_VARIABLE_WRITE, 3);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[1], "11", WE_BASIC_STRING, 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_ASSIGNMENT, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "a2=", WE_VARIABLE_WRITE, 3);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[1], "12", WE_BASIC_STRING, 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[2], CW_ASSIGNMENT, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[2]->Elements[0], "c=", WE_VARIABLE_WRITE, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[2]->Elements[1], "=", WE_BASIC_STRING, 1);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, echoVarTest) {
    char* command = "echo 'abc'$a";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "echo", WE_BASIC_STRING, 4);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 2);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "abc", WE_ESCAPED_STRING, 3);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[1], "$a", WE_VARIABLE_READ, 2);
    DeletePipeExpression(expression);
}

TEST(GrammarTestSuite, patternTest) {
    char* command = "ls {0*,[a-z][a-z][a-z]}";

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    CHECK_PIPE_EXPR(expression, 1);
    CHECK_COMM_EXPR(expression->Commands[0], 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[0], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[0]->Elements[0], "ls", WE_BASIC_STRING, 2);
    CHECK_COMM_WORD(expression->Commands[0]->Words[1], CW_BASIC, 1);
    CHECK_WORD_ELEM(expression->Commands[0]->Words[1]->Elements[0], "{0*,[a-z][a-z][a-z]}", WE_BASIC_STRING, 20);
    DeletePipeExpression(expression);
}
