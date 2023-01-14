#pragma once

/// @brief Path to temporary directory
extern const char* Temp;

/// @brief Path to dev null device
extern const char* DevNull;

/// @brief Opens a pipe for reading
/// @param path Path to the named pipe
/// @return Same as open()
int open_in_pipe(char *path);

/// @brief Opens a pipe for writing
/// @param path Path to the named pipe
/// @return Same as open()
int open_out_pipe(char *path);

/// @brief Creates a new named pipe with attached in and out descriptors
/// @param pipe_in Pointer to the in pipe descriptor
/// @param pipe_out Pointer to the out pipe descriptor
/// @return 0 on success, negative on error
int create_pipe_pair(int* pipe_in, int* pipe_out);

/// @brief Waits for descriptor to contain data or close
/// @param fd File descriptor to watch
/// @return 0 on data or close, negative on error
int wait_for_data(int *fd);

/// @brief Signature of a function that can be executed as a command
typedef int (*InternalCommand)(const char *file, char *const *argv, char *const *envp);

/// @brief Creates a child process to run a command
/// @param pipe_in Stdin of the child process, will be closed
/// @param pipe_out Stdout and stderr of the child process, will be closed
/// @param callback Optional callback that will run instead of the default execvpe
/// @param file Same as execvpe
/// @param argv Same as execvpe
/// @param envp Same as execvpe
/// @return Child PID on success, negative on error
int attach_command(int pipe_in, int pipe_out, InternalCommand callback, const char *file, char *const *argv, char *const *envp);
