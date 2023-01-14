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

int commandB(const char *file, char *const *argv, char *const *envp) {
    return 0;
}

int commandC(const char *file, char *const *argv, char *const *envp) {
    return 0;
}

int main()
{
    // while (1)
    // {
    //     int res = Scan();
    //     printf("%d\n", res);
    // }

    char *args[] = {"Arg1", "Arg2", NULL};
    int res;
    int pipe_in, pipe_out;
    res = create_pipe_pair(&pipe_in, &pipe_out);
    pid_t proc = attach_command(pipe_in, std_out_pipe(), dump_info_command, "Dump Info Command", args, NULL);
    char buf[] = "Hello from STDIN!\n";
    write(pipe_out, buf, sizeof buf);
    wait_for_child(proc);

    return 0;
}
