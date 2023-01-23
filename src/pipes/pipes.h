#pragma once

#include <sys/types.h>

/// @brief Path to temporary directory
extern const char* Temp;

/// @brief Path to /dev/null device
extern const char* DevNull;

/// @brief Opens a read descriptor on a file
/// @return Same as open()
int file_in(char* file);

/// @brief Opens a write descriptor on a file, creates it if necessary, truncates if already exists
/// @return Same as open()
int file_out(char* file);

/// @brief Creates a new named pipe with attached in and out descriptors
/// @param pipe_in Pointer to the in pipe descriptor
/// @param pipe_out Pointer to the out pipe descriptor
/// @return 0 on success, negative on error
int create_pipe_pair(int *pipe_in, int *pipe_out);

/// @brief Waits for descriptor to contain data or close
/// @param fd File descriptor to watch
/// @return 0 on data or close, negative on error
int wait_fd_ready(int fd);

/// @brief Signature of a function that can be executed as a command
typedef int (*InternalCommand)(const char *file, char *const *argv);

/// @brief Creates a child process to run a command
/// @param pipe_in Stdin of the child process, will be closed
/// @param pipe_out Stdout and stderr of the child process, will be closed
/// @param callback Optional callback that will run instead of the default execvpe
/// @param file Same as execvpe
/// @param argv Same as execvpe
/// @param envp Same as execvpe
/// @return Child PID on success, negative on error
int attach_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp);

/// @brief Check if any of the children finished or crashed
/// @param children List of pids
/// @return 0 if still running or exited, -1 on crashed
int check_children(pid_t* children);

/// @brief Wait for all of the children to finish
/// @param children List of pids
/// @return 0 if exited, -1 on crashed
int wait_for_children(pid_t* children);

/// @brief Signal all of the children to die
/// @param children List of pids
void kill_children(pid_t *children);
