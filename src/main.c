#include <stdio.h>
#include "lib/logger.c"
#include "lib/mmem.h"
#include "interpreter/cli.h"

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

int parseArgs(const int ac, const char** av, int* isBatch)
{
    int i = 1;
    while (i < ac) {
        if (strcmp(av[i], "-c") == 0) {
            if (++i < ac) {
                *isBatch = i;
                return 0;
            } else {
                fprintf(stderr, "Missing command.\n");
                return 1;
            }
        }
        i++;
    }
    return 0;
}


int main(const int argumentsCount, const char *argumentsValues[]) {
    init_logger();
    log_info("Shell started.");

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
    if (parseArgs(argumentsCount, argumentsValues, &isBatch) != 0)
        return 1;

    AutoEntry(GlobalMemContext);
    superprint(printf);

    interface(isBatch, argumentsValues);

    AutoExit(GlobalMemContext);
    return 0;
}
