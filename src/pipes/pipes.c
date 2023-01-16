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
#include "../lib/logger.h"

const char* Temp = "/tmp";
const char* DevNull = "/dev/null";

int file_in(char* file) {
    log_info("In file: '%s'", file);
    return open(file, O_RDONLY);
}

int file_out(char* file) {
    log_info("Out file: '%s'", file);
    return open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
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

    unwrap(mkfifo(fifo_path, S_IRUSR | S_IWUSR));
    int pipe_in_res = unwrap(open(fifo_path, O_RDONLY | O_NONBLOCK | __O_CLOEXEC));
    int pipe_out_res = unwrap(open(fifo_path, O_WRONLY | O_NONBLOCK | __O_CLOEXEC));
    
    *pipe_in = pipe_in_res;
    *pipe_out = pipe_out_res;
    return 0;
}

int wait_fd_ready(int fd) {
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
    };

    unwrap(poll(&pfd, 1, -1));
    
    if (pfd.events & POLLERR)
        panic("Fifo has no reader");
    
    if (pfd.events & POLLNVAL)
        panic("Fifo file descriptor invalid");

    return 0;
}

int exec_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    // Overwrite standard pipes and close the provided ones if necessary
    if (pipe_in != STDIN_FILENO) {
        unwrap(dup2(pipe_in, STDIN_FILENO));
        unwrap(close(pipe_in));
    }
    if (pipe_out != STDOUT_FILENO) {
        unwrap(dup2(pipe_out, STDOUT_FILENO));
        unwrap(close(pipe_out));
    }

    // Run internal or external command
    if (callback != NULL)
        expect(callback(file, argv, envp), "Internal command failed");
    else
        unwrap(execvpe(file, argv, envp));
    
    return 0;
}

int attach_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    // Ensure provided pipes are inherited
    if (pipe_in != STDIN_FILENO) 
        unwrap(fcntl(pipe_in, F_SETFD, 0));
    if (pipe_out != STDOUT_FILENO)
        unwrap(fcntl(pipe_out, F_SETFD, 0));
    
    // Run internal setup
    int pid = unwrap(fork());
    if (pid == 0)
        exit(exec_command(pipe_in, pipe_out, callback, file, argv, envp));

    // Close pipes so they don't get inherited later
    if (pipe_in != STDIN_FILENO)
        unwrap(close(pipe_in));
    if (pipe_out != STDOUT_FILENO)
        unwrap(close(pipe_out));
    
    return pid;
}

int wait_for_child(pid_t pid) {
    int status;
    return unwrap(waitpid(pid, &status, 0));
}
