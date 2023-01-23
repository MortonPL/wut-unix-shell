#include "tau/tau/tau.h"
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "../src/interpreter/interpreter.h"


const int CHILD_PID = -1;
#define CWD_SIZE 2048

void runInterpreter(char* command) {
    ExecutionCtx ectx = {
        .next_pipe_in = -1,
    };

    LexerState lexerState;
    InitializeLexer(&lexerState, command);
    PipeExpression *expression = ReadPipeExpression(&lexerState);
    while (expression != NULL) {
        interpret(expression, &ectx);
        DeletePipeExpression(expression);
        expression = ReadPipeExpression(&lexerState);
    }
}

TEST(InterpreterTestSuite, internalFunTest) {
    char* command = "cd CMakeFiles";
    char oldPath[CWD_SIZE] = {0};
    CHECK_NOT_NULL(getcwd(oldPath, CWD_SIZE));
    strcat(oldPath, "/CMakeFiles");

    runInterpreter(command);

    char newPath[CWD_SIZE] = {0};
    CHECK_NOT_NULL(getcwd(newPath, CWD_SIZE));
    CHECK_STREQ(oldPath, newPath);
}

TEST(InterpreterTestSuite, externalFunTest) {
    remove("aVeryTempDir");

    runInterpreter("mkdir aVeryTempDir");

    DIR* dir = opendir("aVeryTempDir");
    CHECK_NOT_NULL(dir);
    closedir(dir);

    remove("aVeryTempDir");
}
