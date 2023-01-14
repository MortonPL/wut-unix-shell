#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib/mmem.h"
#include "parser/parser.h"
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

int dump_info_command(const char *file, char *const *argv, char *const *envp) {
    printf("%s\n", file);

    if (argv != NULL) {
        char *arg = argv[0];
        for (int i = 1; arg != NULL; i++) {
            printf("%s\n", arg);
            arg = argv[i];
        }
    }

    if (envp != NULL) {
        char *env = envp[0];
        for (int i = 1; env != NULL; i++) {
            printf("%s\n", env);
            env = envp[i];
        }
    }

    char buf[512];
    int bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buf, sizeof buf)) > 0)
        write(STDOUT_FILENO, buf, bytes_read);

    return 0;
}

int delayed_write_command(const char *file, char *const *argv, char *const *envp) {
    sleep(3);

    printf("Delayed message.\n");

    return 0;
}

int delayed_close_command(const char *file, char *const *argv, char *const *envp) {
    sleep(3);

    close(STDOUT_FILENO);

    return 0;
}

int main()
{
    // while (1)
    // {
    //     int res = Scan();
    //     printf("%d\n", res);
    // }

    // Basic info dump
    // char *args[] = {"Arg1", "Arg2", NULL};
    // extern char **environ;
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd = attach_command(pipe_in, std_out_pipe(), dump_info_command, "Dump Info Command", args, environ);
    // char buf[] = "Hello from STDIN!\n";
    // write(pipe_out, buf, sizeof buf);
    // wait_for_child(cmd);

    // Wait for data on pipe
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd1 = attach_command(null_in_pipe(), pipe_out, delayed_write_command, "Delayed Write Command", NULL, NULL);
    // wait_for_data(pipe_in);
    // pid_t cmd2 = attach_command(pipe_in, std_out_pipe(), dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd1);
    // wait_for_child(cmd2);

    // Wait for closed pipe
    // int pipe_in, pipe_out;
    // create_pipe_pair(&pipe_in, &pipe_out);
    // pid_t cmd1 = attach_command(null_in_pipe(), pipe_out, delayed_close_command, "Delayed Close Command", NULL, NULL);
    // wait_for_data(pipe_in);
    // pid_t cmd2 = attach_command(pipe_in, std_out_pipe(), dump_info_command, "Dump Info Command", NULL, NULL);
    // wait_for_child(cmd1);
    // wait_for_child(cmd2);

    // System command call
    char *args[] = {"echo", "Current", "path", getenv("PWD"), NULL};
    pid_t cmd = attach_command(null_in_pipe(), std_out_pipe(), NULL, "echo", args, NULL);
    wait_for_child(cmd);

    return 0;
}
