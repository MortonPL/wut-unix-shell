#include "tau/tau/tau.h"
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "../src/interpreter/interpreter.h"


const int CHILD_PID = -1;
const int COMMAND_SIZE = 256;

void runInterpreter(char* command) {
    // MemContext context = MakeContext();
    // Env env;
    // env.cwd = (char*) AutoMalloc(context, COMMAND_SIZE, free);
    // env.childPid = &CHILD_PID;
    // env.variableCount = 0;
    // if (getcwd(env.cwd, COMMAND_SIZE) == NULL) {
    //     exit(EXIT_FAILURE);
    // }
    // strcpy(env.previousWorkingDirectory, env.cwd);

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

    // AutoExit(context);
}

TEST(InterpreterTestSuite, internalFunTest) {
    char* command = "cd logs";
    char oldPath[256] = {0};
    getcwd(oldPath, COMMAND_SIZE);
    strcat(oldPath, "/logs");

    runInterpreter(command);

    char newPath[256] = {0};
    getcwd(newPath, COMMAND_SIZE);
    CHECK_STREQ(oldPath, newPath);
}

TEST(InterpreterTestSuite, externalFunTest) {
    DIR* dir = opendir("aVeryTempDir");
    CHECK_EQ(ENOENT, errno);

    runInterpreter("mkdir aVeryTempDir");

    dir = opendir("aVeryTempDir");
    CHECK_NOT_NULL(dir);
    closedir(dir);

    runInterpreter("rm -r aVeryTempDir");
}

TEST(InterpreterTestSuite, redirectingTest) {
    runInterpreter("echo aaa > aVeryTempFile.txt");
    int fileDescriptor = open("aVeryTempFile.txt", O_RDONLY);
    char buffer[64] = {0}; 
    read(fileDescriptor, buffer, 64);
    CHECK_STREQ(buffer, "aaa");
    close(fileDescriptor);
    runInterpreter("rm aVeryTempFile");
}

TEST(InterpreterTestSuite, fileAccessTest) {
    fclose(fopen("aVeryTempFile.txt", "r"));
    CHECK_EQ(chmod("aVeryTempFile.txt", 000), 0);
    runInterpreter("cat aVeryTempFile.txt");
}
