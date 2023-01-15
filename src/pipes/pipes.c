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

int devnull_in() {
    return open(DevNull, O_WRONLY);
}

int devnull_out() {
    return open(DevNull, O_WRONLY);
}

int file_in(char* file) {
    open(file, O_RDONLY);
}

int file_out(char* file) {
    open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
}

void generate_fifo_filename(char* path_buf) {
    uuid_t uuid_bytes;
    strncpy(path_buf, Temp, 4);
    path_buf[4] = '/';
    uuid_generate_random(uuid_bytes);
    uuid_unparse(uuid_bytes, &path_buf[5]);
}

int create_pipe_pair(int* pipe_in, int* pipe_out) {
    char fifo_path[42];
    generate_fifo_filename(fifo_path);
    
    int mkfifo_res = mkfifo(fifo_path, S_IRUSR | S_IWUSR);
    RETURN_ON_ERR(mkfifo_res, -1);
    int pipe_in_res = open(fifo_path, O_RDONLY | O_NONBLOCK | __O_CLOEXEC);
    RETURN_ON_ERR(pipe_in_res, -2);
    int pipe_out_res = open(fifo_path, O_WRONLY | O_NONBLOCK | __O_CLOEXEC);
    RETURN_ON_ERR(pipe_out_res, -2);
    
    *pipe_in = pipe_in_res;
    *pipe_out = pipe_out_res;
    return 0;
}

int wait_fd_ready(int fd) {
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
    // Ensure provided pipes are inherited
    if (pipe_in != STDIN_FILENO) {
        res = fcntl(pipe_in, F_SETFD, 0);
        RETURN_ON_ERR(res, -1);
    }
    if (pipe_in != STDOUT_FILENO) {
        res = fcntl(pipe_out, F_SETFD, 0);
        RETURN_ON_ERR(res, -1);
    }
    // Run internal setup
    int fork_res = fork();
    if (fork_res == 0) {
        int child_res = exec_command(pipe_in, pipe_out, callback, file, argv, envp);
        exit(child_res);
    }
    RETURN_ON_ERR(fork_res, -2);
    // Close pipes so they don't get inherited later
    if (pipe_in != STDIN_FILENO) {
        res = close(pipe_in);
        RETURN_ON_ERR(res, -3);
    }
    if (pipe_in != STDOUT_FILENO) {
        res = close(pipe_out);
        RETURN_ON_ERR(res, -3);
    }
    return fork_res;
}

int exec_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    int res;
    // Overwrite standard pipes and close the provided ones if necessary
    if (pipe_in != STDIN_FILENO) {
        res = dup2(pipe_in, STDIN_FILENO);
        RETURN_ON_ERR(res, -1);
        res = close(pipe_in);
        RETURN_ON_ERR(res, -2);
    }
    if (pipe_out != STDOUT_FILENO) {
        res = dup2(pipe_out, STDOUT_FILENO);
        RETURN_ON_ERR(res, -1);
        res = close(pipe_out);
        RETURN_ON_ERR(res, -2);
    }
    // Run internal or external command
    res = (callback != NULL)
        ? callback(file, argv, envp)
        : execvpe(file, argv, envp);
    RETURN_ON_ERR(res, -3);
    return 0;
}

int wait_for_child(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
    return status;
}
