#define _GNU_SOURCE

#include "pipes.h"
#include "stdio.h"
#include "uuid/uuid.h"
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

#define RETURN_ON_ERR(x, y) if (x < 0) return y

extern const char* Temp = "/tmp";
extern const char* DevNull = "/dev/null";

void generate_unique_temporary_filename(char* path_buf) {
    uuid_t uuid_bytes;
    strncpy(path_buf, Temp, 4);
    path_buf[4] = '/';
    uuid_generate_random(uuid_bytes);
    uuid_unparse(uuid_bytes, &path_buf[5]);
}

int open_in_pipe(char* path) {
    return open(path, O_RDONLY | O_NONBLOCK | __O_CLOEXEC);
}

int open_out_pipe(char* path) {
    return open(path, O_WRONLY | O_NONBLOCK | __O_CLOEXEC);
}

int std_in_pipe() {
    return dup(STDIN_FILENO);
}

int std_out_pipe() {
    return dup(STDOUT_FILENO);
}

int null_in_pipe() {
    return open(DevNull, O_WRONLY | __O_CLOEXEC);
}

int null_out_pipe() {
    return open(DevNull, O_WRONLY | __O_CLOEXEC);
}

int create_pipe_pair(int* pipe_in, int* pipe_out) {
    char fifo_path[42];
    generate_unique_temporary_filename(fifo_path);
    
    int mkfifo_res = mkfifo(fifo_path, S_IRUSR | S_IWUSR);
    RETURN_ON_ERR(mkfifo_res, -1);
    
    int pipe_in_res = open_in_pipe(fifo_path);
    RETURN_ON_ERR(pipe_in_res, -2);
    
    int pipe_out_res = open_out_pipe(fifo_path);
    RETURN_ON_ERR(pipe_out_res, -3);
    
    *pipe_in = pipe_in_res;
    *pipe_out = pipe_out_res;
    return 0;
}

int wait_for_data(int fd) {
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events |= POLLIN;

    int poll_res = poll(&pfd, 1, -1);
    RETURN_ON_ERR(poll_res, -1);
    
    if (!(pfd.revents & (POLLIN | POLLHUP)))
        return -2;

    return 0;
}

int attach_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    int res;
    res = fcntl(pipe_in, F_SETFD, 0);
    RETURN_ON_ERR(res, -1);
    res = fcntl(pipe_out, F_SETFD, 0);
    RETURN_ON_ERR(res, -1);

    int fork_res = fork();
    if (fork_res == 0) {
        int child_res = exec_command(pipe_in, pipe_out, callback, file, argv, envp);
        exit(child_res);
    }
    RETURN_ON_ERR(fork_res, -2);

    res = close(pipe_in);
    RETURN_ON_ERR(res, -3);
    res = close(pipe_out);
    RETURN_ON_ERR(res, -3);

    return fork_res;
}

int exec_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    int res;
    res = dup2(pipe_in, STDIN_FILENO);
    RETURN_ON_ERR(res, -1);
    res = dup2(pipe_out, STDOUT_FILENO);
    RETURN_ON_ERR(res, -1);
    res = dup2(pipe_out, STDERR_FILENO);
    RETURN_ON_ERR(res, -1);
    res = close(pipe_in);
    RETURN_ON_ERR(res, -2);
    res = close(pipe_out);
    RETURN_ON_ERR(res, -2);

    if (callback != NULL)
        res = callback(file, argv, envp);
    else
        res = execvpe(file, argv, envp);
    RETURN_ON_ERR(res, -3);

    return 0;
}

int wait_for_child(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}
