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

    errreturn(logoserr(mkfifo(fifo_path, S_IRUSR | S_IWUSR)));
    int pipe_in_res = logoserr(open(fifo_path, O_RDONLY | O_NONBLOCK | __O_CLOEXEC));
    errreturn(pipe_in_res);
    int pipe_out_res = logoserr(open(fifo_path, O_WRONLY | O_NONBLOCK | __O_CLOEXEC));
    errreturn(pipe_out_res);

    *pipe_in = pipe_in_res;
    *pipe_out = pipe_out_res;
    return 0;
}

int wait_fd_ready(int fd) {
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
    };

    int res = poll(&pfd, 1, -1);
    if (res < 0) {
        if (errno != EINTR)
            errreturn(logoserr(res));
        else {
            log_info("Poll interrupted");
            return -1;
        }
    }

    if (pfd.events & POLLERR)
        panic("Fifo has no reader");
    
    if (pfd.events & POLLNVAL)
        panic("Fifo file descriptor invalid");

    return 0;
}

int exec_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv) {
    // Overwrite standard pipes and close the provided ones if necessary
    if (pipe_in != STDIN_FILENO) {
        errreturn(logoserr(dup2(pipe_in, STDIN_FILENO)));
        errreturn(logoserr(close(pipe_in)));
    }
    if (pipe_out != STDOUT_FILENO) {
        errreturn(logoserr(dup2(pipe_out, STDOUT_FILENO)));
        errreturn(logoserr(close(pipe_out)));
    }

    // Run internal or external command
    if (callback != NULL)
        errreturn(logerr(callback(file, argv), "Internal command failed"));
    else
        errreturn(logoserr(execvp(file, argv)));
    
    return 0;
}

int attach_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp) {
    // Ensure provided pipes are inherited
    if (pipe_in != STDIN_FILENO) 
        errreturn(logoserr(fcntl(pipe_in, F_SETFD, 0)));
    if (pipe_out != STDOUT_FILENO)
        errreturn(logoserr(fcntl(pipe_out, F_SETFD, 0)));
    
    // Run internal setup
    int pid = logoserr(fork());
    if (pid == 0) {
        char *const *envp_iter = envp;
        while (*envp_iter != NULL)
            putenv(*envp_iter++);
        exit(exec_command(pipe_in, pipe_out, callback, file, argv));
    }

    // Close pipes so they don't get inherited later
    if (pipe_in != STDIN_FILENO)
        errreturn(logoserr(close(pipe_in)));
    if (pipe_out != STDOUT_FILENO)
        errreturn(logoserr(close(pipe_out)));

    log_trace("Spawned child %s as %i", file, pid);
    return pid;
}

int check_children(pid_t* children) {
    log_trace("Checking children");
    while (*children != 0) {
        log_trace("Checking child %i", *children);
        if (*children != 1) {
            int status;
            int pid = logoserr(waitpid(*children, &status, WNOHANG));
            errreturn(pid);
            if (pid != 0 && WIFEXITED(status)) {
                *children = 1;
                int exit_status = WEXITSTATUS(status);
                log_info("Subprocess with pid %i finished with status code %i", pid, exit_status);
                errreturn(exit_status);
            }
        }
        children++;
    }
    return 0;
}

int wait_for_children(pid_t* children) {
    log_trace("Waiting for children");
    while (*children != 0) {
        log_trace("Waiting for child %i", *children);
        if (*children != 1) {
            int status;
            int pid = logoserr(waitpid(*children, &status, 0));
            errreturn(pid);
            if (WIFEXITED(status)) {
                *children = 1;
                int exit_status = WEXITSTATUS(status);
                log_info("Subprocess with pid %i finished with status code %i", pid, exit_status);
                errreturn(exit_status);
            }
        }
        children++;
    }
    return 0;
}

void kill_children(pid_t *children) {
    log_trace("Killing children");
    if (children != NULL) {
        while (*children != 0) {
            if (*children != 1) {
                log_info("Killing subprocess with pid %i", *children);
                logoserr(kill(*children, SIGKILL));
            }
            children++;
        }
    }
}
