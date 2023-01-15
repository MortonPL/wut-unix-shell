#include "logger.h"

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

int ensure_logs_folder_exists() {
    int res = mkdir("logs", S_IRWXU | S_IRWXG | S_IRWXO);
    if (res < 0)
        if (errno != EEXIST)
            return res;
    return 0;
}

void current_time_string(char* buffer) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm);
}

int init_logger() {
    int res = ensure_logs_folder_exists();
    if (res < 0)
        return res;

    char log_file_path[64] = "logs/";
    current_time_string(&log_file_path[strlen(log_file_path)]);
    int log_mode;
    #ifdef DEBUG
        strcat(log_file_path, "-debug.txt");
        log_mode = LOG_TRACE;
    #else
        strcat(log_file_path, "-release.txt");
        log_mode = LOG_ERROR;
    #endif
    FILE *log_file = fopen(log_file_path, "w");
    log_add_fp(log_file, log_mode);
    return 0;
}

void __panic(const char *file, int line, const char *fmt, ...) {
    log_Event ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .level = LOG_FATAL,
    };
    va_start(ev.ap, fmt);
    vlog_log(ev);
    va_end(ev.ap);
    exit(-1);
}

int __expect(int status_code, const char *file, int line, const char *fmt, ...) {
    if (status_code < 0) {
        log_Event ev = {
            .fmt   = fmt,
            .file  = file,
            .line  = line,
            .level = LOG_FATAL,
        };
        va_start(ev.ap, fmt);
        vlog_log(ev);
        va_end(ev.ap);
        exit(-1);
    }
    return status_code;
}

int __unwrap(int status_code, const char *file, int line) {
    return __expect(status_code, file, line, "OS error: %s", strerror(errno));
}
