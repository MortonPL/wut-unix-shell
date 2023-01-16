#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/mmem.h"
#include "parser/interface.h"
#include "parser/structures.h"
#include "pipes/pipes.h"

typedef int (*helloer)(const char* msg, ...);

void closer(void* pFd)
{
    int fd = (int)*(long*)pFd;
    close(fd);
}

void superprint(helloer fun)
{
    // stupid little demo of stupid little automatic memory management
    // also a stupid little demo of function pointers, they might be useful (TKOM vibes)
    MemContext ctx = MakeContext();
    char* x = (char*)AutoMalloc(ctx, 6, free);
    x[0] = 'U';
    x[1] = 'X';
    x[2] = 'P';
    x[3] = '1';
    x[4] = 'A';
    x[5] = '\0';
    AutoMalloc(ctx, 100, free);

    int fd = *(int*)AutoInsert(ctx, open("/dev/zero", O_RDONLY), closer);
    (void)fd;

    fun("Hello %s!\n", x);
    AutoExit(ctx);
    // won't leak!
}

void print_content(char *const *content) {
    if (content != NULL) {
        char *subcontent = content[0];
        for (int i = 1; subcontent != NULL; i++) {
            printf("%s\n", subcontent);
            subcontent = content[i];
        }
    }
}

void move_in_to_out() {
    char buf[512];
    ssize_t bytes_read, bytes_wrote;
    while ((bytes_read = read(STDIN_FILENO, buf, sizeof buf)) > 0) {
        bytes_wrote = write(STDOUT_FILENO, buf, (size_t) bytes_read);
        while (bytes_wrote < bytes_read)
            bytes_wrote += write(STDOUT_FILENO, &buf[bytes_wrote], (size_t) (bytes_read - bytes_wrote));
    }
}

int dump_info_command(const char *file, char *const *argv, char *const *envp) {
    printf("%s\n", file);
    print_content(argv);
    print_content(envp);
    fflush(stdout);
    move_in_to_out();
    return 0;
}

int delayed_write_command(const char *file, char *const *argv, char *const *envp) {
    sleep(3);
    printf("Delayed message.\n");
    (void)file;
    (void)argv;
    (void)envp;
    return 0;
}

int delayed_close_command(const char *file, char *const *argv, char *const *envp) {
    sleep(3);
    close(STDOUT_FILENO);
    (void)file;
    (void)argv;
    (void)envp;
    return 0;
}

int ParseArgs(int ac, char** av, int* isBatch)
{
    int i = 1;
    while (i < ac)
    {
        if (strcmp(av[i], "-c") == 0)
        {
            if (++i < ac)
            {
                *isBatch = i;
                return 0;
            }
            else
            {
                fprintf(stderr, "Missing command.\n");
                return 1;
            }
        }
        i++;
    }
    return 0;
}

int main(int ac, char** av)
{
    // Basic info dump
    // char *args[] = {"Arg1", "Arg2", NULL};
    // extern char **environ;
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd = attach_command(pipe_in, STDOUT_FILENO, dump_info_command, "Dump Info Command", args, environ);
    // char buf[] = "Hello from STDIN!\n";
    // write(pipe_out, buf, sizeof buf);
    // wait_for_child(cmd);

    // Wait for data on pipe
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd1 = attach_command(devnull_in(), pipe_out, delayed_write_command, "Delayed Write Command", NULL, NULL);
    // wait_fd_ready(pipe_in);
    // pid_t cmd2 = attach_command(pipe_in, STDOUT_FILENO, dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd1);
    // wait_for_child(cmd2);

    // Wait for closed pipe
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd1 = attach_command(devnull_in(), pipe_out, delayed_close_command, "Delayed Close Command", NULL, NULL);
    // wait_fd_ready(pipe_in);
    // pid_t cmd2 = attach_command(pipe_in, STDOUT_FILENO, dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd1);
    // wait_for_child(cmd2);

    // Wait for null pipe
    // int pipe_in = devnull_in();
    // wait_fd_ready(pipe_in);
    // pid_t cmd = attach_command(pipe_in, STDOUT_FILENO, dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd);

    // System command call
    // char *args[] = {"echo", "Current", "path", getenv("PWD"), NULL};
    // pid_t cmd = attach_command(devnull_in(), STDOUT_FILENO, NULL, "echo", args, NULL);
    // wait_for_child(cmd);

    // Read from file, dump to file
    // int pipe_in = file_in("./lorem.txt");
    // int pipe_out = file_out("./merol.txt");
    // pid_t cmd = attach_command(pipe_in, pipe_out, dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd);
    
    int isBatch = 0;
    if (ParseArgs(ac, av, &isBatch) != 0)
        return 1;

    AutoEntry(GlobalMemContext);
    superprint(printf);
    AutoExit(GlobalMemContext);

    FILE* input = stdin;
    if (isBatch)
        input = fmemopen(av[isBatch], strlen(av[isBatch]), "r");

    const int bufsize = 256;
    char buffer[bufsize];
    while (fgets(buffer, bufsize, input) != NULL) {
        if (strlen(buffer) == 0)
            continue;
        buffer[bufsize - 1] = '\0';
        LexerState lexerState;
        InitializeLexer(&lexerState, buffer);
        PipeExpression *pExpression = ReadPipeExpression(&lexerState);
        while (pExpression != NULL) {
            PrintPipeExpression(pExpression, 0);
            DeletePipeExpression(pExpression);
            pExpression = ReadPipeExpression(&lexerState);
        }
        CleanupLexer(&lexerState);
    }

    return 0;
}
