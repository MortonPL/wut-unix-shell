#include <ctype.h>
#include <glob.h>
#include "cli.h"
#include "../lib/logger.h"
#include "../lib/log.c/src/log.h"

pid_t* children = NULL;
size_t lenv_size = 0;
char **lenv = NULL;

/// @brief Built-in cd command
/// @param file The name of executed file.
/// @param argv Program arguments.
/// @param envp Additional environment variables.
int cd_cmd(const char *file, char* const* argv, char* const* envp) {
    (void)file, (void)envp;
    argv++;
    if (*argv == NULL) {
        fputs("cd failed: missing required argument\n", stderr);
        panic("missing required argument");
    }

    char *new_dir = *argv;

    argv++;
    if (*argv != NULL) {
        fputs("cd failed: too many argument\n", stderr);
        panic("too many arguments");
    }

    errno = 0;
    if (logoserr(chdir(new_dir)) == -1) {
        perror("cd failed");
        return -1;
    }
    return 0;
}

static bool is_identifier(const char *str, long len)
{
    if (len == 0)
        return false;
    if (!isalpha(str[0]) && str[0] != '_')
        return false;
    for (long i = 0; i < len; i++)
        if (!isalnum(str[i]) && str[i] != '_')
            return false;
    return true;
}

static size_t min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static char *copyString(const char *pSource)
{
    size_t strLength = strlen(pSource);
    char *pCopy = (char *)malloc(strLength + 1);
    if (pCopy == NULL)
        return NULL;
    memcpy(pCopy, pSource, strLength + 1);
    return pCopy;
}

/// @brief Built-in export command
/// @param file The name of executed file.
/// @param argv Program arguments.
/// @param envp Additional environment variables.
int export_cmd(const char *file, char* const* argv, char* const* envp) {
    (void)file;
    argv++;
    if (*argv == NULL) {
        fputs("export failed: missing required argument\n", stderr);
        panic("missing required argument");
    }

    while (*argv != NULL) {
        char *eq_pos = strchr(*argv, '=');
        if (eq_pos != NULL && is_identifier(*argv, eq_pos - *argv)) {
            errno = 0;
            char *exported_var = copyString(*argv);
            log_trace("Exporting env: %s", exported_var);
            if (logoserr(putenv(exported_var)) == -1) {
                perror("export failed");
                return -1;
            }
            argv++;
            continue;
        }
        char *const *envp_iter = envp;
        while (*envp_iter != NULL) {
            eq_pos = strchr(*envp_iter, '=');
            if (eq_pos != NULL && memcmp(*argv, *envp_iter, min(strlen(*argv), (size_t)(eq_pos - *envp_iter))) == 0) {
                char *exported_var = copyString(*envp_iter);
                log_trace("Exporting env: %s", exported_var);
                if (logoserr(putenv(exported_var)) == -1) {
                    perror("export failed");
                    return -1;
                }
                break;
            }
            envp_iter++;
        }
        argv++;
    }

    return 0;
}

#define MAX_PATH 1024

/// @brief Built-in pwd command
/// @param file The name of executed file.
/// @param argv Program arguments.
int pwd_cmd(const char *file, char* const* argv) {
    (void)file, (void)argv;
    char buf[MAX_PATH];

    char *res = getcwd(buf, MAX_PATH);
    if (res != buf) {
        perror("pwd failed");
        panic(OsErrorMessage, strerror(errno));
    }

    printf("%s\n", buf);
    return 0;
}

/// @brief Built-in echo command
/// @param file The name of executed file.
/// @param argv Program arguments.
int echo_cmd(const char *file, char* const* argv) {
    (void)file;
    argv++;
    while (*argv != NULL) {
        printf("%s ", *argv);
        argv++;
    }
    printf("\n");
    return 0;
}

/// @brief Print array of strings
/// @param fmt Format string
/// @param s Array of strings
void print_str_arr(char *fmt, char **s) {
    if (s != NULL)
        while (*s != NULL) {
            log_trace(fmt, *s);
            s += 1;
        }
}

/// @brief Free array of strings
/// @param s Array of strings
void free_str_arr(char **s) {
    if (s != NULL)
        while (*s != NULL) {
            free(*s);
            s += 1;
        }
}

/// @brief Counts amount of args and eenv in a command
/// @param cmd_expr Command expression to count from
/// @param args_count Pointer to args count
/// @param eenv_count Pointer to eenv count
void subprocess_args_eenv_counts(CommandExpression *cmd_expr, size_t *args_count, size_t *eenv_count) {
    CommandWord **cmd_words = cmd_expr->Words;
    while (*cmd_words != NULL) {
        CommandWord *cmd_word = *cmd_words++;
        switch (cmd_word->Type) {
            case CW_ASSIGNMENT:
                (*eenv_count)++;
                break;
            case CW_BASIC:
                (*args_count)++;
                break;
            default:
                break;
        }
    }
}

extern char **environ;

/// @brief Allocate space for command context
/// @param cctx Command context
/// @param args_count Args count
/// @param eenv_count Eenv count
CommandCtx* subprocess_allocate(CommandCtx* cctx, size_t args_count, size_t eenv_count) {
    cctx->args = malloc((args_count + 1) * sizeof(char*));
    nullreturn(cctx->args);
    (cctx->args)[args_count] = NULL;
    cctx->eenv = malloc((eenv_count + 1) * sizeof(char*));
    nullreturn(cctx->eenv);
    (cctx->eenv)[eenv_count] = NULL;
    return cctx;
}

/// @brief Returns the value of a key in environment
/// @param env Command local environment
/// @param key Key to look for
/// @return The value
char* env_get(char **env, char *key) {
    size_t len = strlen(key);
    char **env_iter = env;
    while (*env_iter != NULL) {
        if (strncmp(key, *env_iter, len) == 0 && (*env_iter)[len] == '=') {
            return *env_iter + len + 1;
        }
        env_iter++;
    }
    if (lenv != NULL) {
        char **lenv_iter = lenv;
        while (*lenv_iter != NULL) {
            if (strncmp(key, *lenv_iter, len) == 0 && (*lenv_iter)[len] == '=') {
                return *lenv_iter + len + 1;
            }
            lenv_iter++;
        }
    }
    env_iter = environ;
    while (*env_iter != NULL) {
        if (strncmp(key, *env_iter, len) == 0 && (*env_iter)[len] == '=') {
            return *env_iter + len + 1;
        }
        env_iter++;
    }
    return NULL;
}

// does not append null terminator
static void memcpySlashEscaped(char *pDest, const char *pSource, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        pDest[2 * i] = '\\';
        pDest[2 * i + 1] = pSource[i];
    }
}

static void getCommandWordLength(CommandCtx* cctx, CommandWord *word, size_t *baseLen, size_t *slashEscapedLen)
{
    *baseLen = 0;
    *slashEscapedLen = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        if ((*els)->Type == WE_VARIABLE_READ) {
            char *env_var = env_get(cctx->eenv, (*els)->Value + 1);
            if (env_var != NULL) {
                size_t var_len = strlen(env_var);
                *baseLen += var_len;
                *slashEscapedLen += var_len;
            }
        } else {
            *baseLen += (*els)->Length;
            *slashEscapedLen += (*els)->Length * (((*els)->Type == WE_ESCAPED_STRING) ? 2 : 1);
        }
        els += 1;
    }
}

/// @brief Process a single basic word
/// @param cctx Command context
/// @param word Command word
/// @param returnedCount Count of returned strings
/// @param doGlob Should do glob?
/// @return Processed word
char **process_word(CommandCtx* cctx, CommandWord *word, size_t *returnedCount, bool doGlob) {
    size_t len = 0, escapedLen = 0;
    getCommandWordLength(cctx, word, &len, &escapedLen);

    // Fill
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;
    size_t offset = 0;

    char *escapedOutbuf = malloc(escapedLen + 1);
    escapedOutbuf[escapedLen] = 0;
    size_t escapedOffset = 0;

    WordElement **els = word->Elements;
    while (*els != NULL) {
        if ((*els)->Type == WE_VARIABLE_READ) {
            char *env_var = env_get(cctx->eenv, (*els)->Value + 1);
            if (env_var != NULL) {
                size_t var_len = strlen(env_var);
                memcpy(&outbuf[offset], env_var, var_len);
                memcpy(&escapedOutbuf[escapedOffset], env_var, var_len);
                offset += var_len;
                escapedOffset += var_len;
            }
        } else if ((*els)->Type == WE_ESCAPED_STRING) {
            memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
            memcpySlashEscaped(&escapedOutbuf[escapedOffset], (*els)->Value, (*els)->Length);
            offset += (*els)->Length;
            escapedOffset += (*els)->Length * 2;
        } else {
            memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
            memcpy(&escapedOutbuf[escapedOffset], (*els)->Value, (*els)->Length);
            offset += (*els)->Length;
            escapedOffset += (*els)->Length;
        }

        els += 1;
    }
    log_trace("Processed word: %s", outbuf);
    log_trace("Processed escaped word: %s", escapedOutbuf);

    char **pReturned = NULL;
    bool globMatched = false;
    glob_t globbuf = {0};
    if (doGlob) {
        if ((globMatched = (glob(escapedOutbuf, 0, NULL, &globbuf) == 0))) {
            (*returnedCount) = globbuf.gl_pathc;
            char **pOriginal = globbuf.gl_pathv;
            pReturned = (char **)malloc(sizeof(char *) * (globbuf.gl_pathc + 1));
            pReturned[globbuf.gl_pathc] = NULL;
            for (size_t i = 0; i < globbuf.gl_pathc; i++) {
                log_trace("Copying glob element: %s", pOriginal[i]);
                pReturned[i] = copyString(pOriginal[i]);
            }
        }
    }
    globfree(&globbuf);
    free(escapedOutbuf);

    if (globMatched) {
        free(outbuf);
    } else {
        (*returnedCount) = 1;
        pReturned = (char **)malloc(sizeof(char *) * 2);
        pReturned[0] = outbuf;
        pReturned[1] = NULL;
    }
    return pReturned;
}

/// @brief Fill command ctx with information from command expression
/// @param cctx Context to fill
/// @param cmd_expr Command expression to consume
CommandExpression *process_command(CommandCtx* cctx, CommandExpression *cmd_expr) {
    size_t args_count = 0, eenv_count = 0;
    subprocess_args_eenv_counts(cmd_expr, &args_count, &eenv_count);
    nullreturn(subprocess_allocate(cctx, args_count, eenv_count));

    size_t args_i = 0, eenv_i = 0;
    CommandWord **words = cmd_expr->Words;
    while (*words != NULL) {
        CommandWord *word = *words++;
        size_t returnedCount = 0;
        char **pArgs = NULL;
        switch (word->Type) {
            case CW_ASSIGNMENT:
                pArgs = process_word(cctx, word, &returnedCount, args_i > 0);
                nullreturn(pArgs);
                cctx->eenv[eenv_i++] = pArgs[0];
                free(pArgs);
                break;
            case CW_BASIC: {
                pArgs = process_word(cctx, word, &returnedCount, args_i > 0);
                nullreturn(pArgs);
                if (returnedCount > 1) {
                    args_count += returnedCount - 1;
                    cctx->args = (char **)realloc(cctx->args, sizeof(char *) * (args_count + 1));
                    (cctx->args)[args_count] = NULL;
                }
                for (size_t i = 0; i < returnedCount; i++)
                    cctx->args[args_i++] = pArgs[i];
                free(pArgs);
                break;
            }
            case CW_REDIRECTION_IN:
                if (cctx->redir_in != NULL)
                    free(cctx->redir_in);
                pArgs = process_word(cctx, word, &returnedCount, false);
                nullreturn(pArgs);
                cctx->redir_in = pArgs[0];
                free(pArgs);
                break;
            case CW_REDIRECTION_OUT:
                if (cctx->redir_out != NULL)
                    free(cctx->redir_out);
                pArgs = process_word(cctx, word, &returnedCount, false);
                nullreturn(pArgs);
                cctx->redir_out = pArgs[0];
                free(pArgs);
                break;
        }
    }
    return cmd_expr;
}

/// @brief Free command ctx struct
/// @param cctx Pointer to the struct
void free_command(CommandCtx* cctx) {
    if (cctx == NULL) return;
    if (cctx->args != NULL) {
        free_str_arr(cctx->args);
        cctx->args = NULL;
    }
    if (cctx->eenv != NULL) {
        free_str_arr(cctx->eenv);
        cctx->eenv = NULL;
    }
    if (cctx->redir_in != NULL) {
        free(cctx->redir_in);
        cctx->redir_in = NULL;
    }
    if (cctx->redir_out != NULL) {
        free(cctx->redir_out);
        cctx->redir_out = NULL;
    }
}

int run_command(ExecutionCtx* ectx, CommandCtx* curr_cctx, CommandCtx* next_cctx) {
    log_trace("=== Running command:");
    print_str_arr("Arg: %s", curr_cctx->args);
    print_str_arr("Env: %s", curr_cctx->eenv);

    int pipe_in, pipe_out;

    // Configure in pipe
    if (ectx->next_pipe_in > 0) {
        pipe_in = ectx->next_pipe_in;
        log_trace("Input piped from previous process");
    }
    else if (curr_cctx->redir_in != NULL) {
        pipe_in = logoserr(file_in(curr_cctx->redir_in));
        errreturn(pipe_in);
        log_trace("Input piped from %s", curr_cctx->redir_in);
    }
    else if (ectx->next_pipe_in == 0) {
        pipe_in = logoserr(file_in((char *)DevNull));
        errreturn(pipe_in);
        log_trace("Input piped from %s", DevNull);
    }
    else {
        pipe_in = STDIN_FILENO;
        log_trace("Input piped from standard input");
    }
    
    // Configure out pipe
    if (curr_cctx->redir_out != NULL) {
        pipe_out = logoserr(file_out(curr_cctx->redir_out));
        errreturn(pipe_out);
        ectx->next_pipe_in = 0;
        log_trace("Output piped to %s", curr_cctx->redir_out);
    }
    else if (next_cctx == NULL) {
        pipe_out = STDOUT_FILENO;
        ectx->next_pipe_in = -1;
        log_trace("Output piped to standard output");
    }
    else if (next_cctx->redir_in != NULL) {
        pipe_out = logoserr(file_out((char *)DevNull));
        errreturn(pipe_out);
        ectx->next_pipe_in = 0;
        log_trace("Output piped to %s", DevNull);
    }
    else {
        errreturn(logoserr(create_pipe_pair(&(ectx->next_pipe_in), &pipe_out)));
        log_trace("Output piped to next process");
    }
    log_trace("===");

    // Check existence of command
    char *cmd = curr_cctx->args[0];
    if (cmd == NULL) {
        if (curr_cctx->eenv[0] != NULL) {
            // No command, just shell process-local env vars
            char **env_iter = curr_cctx->eenv;
            int i = 0;
            while (*env_iter != NULL) {
                size_t key_len = (size_t)(strchr(*env_iter, '=') - *env_iter);
                char **lenv_iter = lenv;
                if (lenv != NULL) {
                    while (*lenv_iter != NULL && (strncmp(*lenv_iter, *env_iter, key_len) != 0 || (*lenv_iter)[key_len] != '=')) {
                        lenv_iter++;
                    }
                }
                if (lenv_iter != NULL && *lenv_iter != NULL) {
                    free(*lenv_iter);
                    *lenv_iter = env_iter[i];
                } else {
                    lenv_size++;
                    lenv = realloc(lenv, (lenv_size + 1) * sizeof(char *));
                    lenv[lenv_size] = NULL;
                    lenv[lenv_size - 1] = env_iter[i];
                }
                env_iter[i] = NULL;
                env_iter++;
                i++;
            }
            return 0;
        } else {
            printf("no command provided");
            panic("no command provided");
        }
    }

    // Run commands
    pid_t pid = 0;
    if (strcmp(cmd, "cd") == 0) {
        log_info("Running internal command '%s'", cmd);
        errreturn(logerr(cd_cmd(cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command"));
    } else if (strcmp(cmd, "export") == 0) {
        log_info("Running internal command '%s'", cmd);
        errreturn(logerr(export_cmd(cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command"));
    } else if (strcmp(cmd, "echo") == 0) {
        log_info("Running internal command '%s'", cmd);
        pid = logerr(attach_command(pipe_in, pipe_out, echo_cmd, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command");
        errreturn(pid);
    } else if (strcmp(cmd, "pwd") == 0) {
        log_info("Running internal command '%s'", cmd);
        pid = logerr(attach_command(pipe_in, pipe_out, pwd_cmd, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command");
        errreturn(pid);
    } else {
        log_info("Running external command '%s'", cmd);
        pid = logerr(attach_command(pipe_in, pipe_out, NULL, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run external command");
        errreturn(pid);
    }

    if (pid != 0) {
        int child_idx = 0;
        pid_t *child_iter = children;
        while (*child_iter > 1) {
            child_idx++;
            child_iter++;
        }
        children[child_idx] = pid;
    }

    if (ectx->next_pipe_in > 0)
        errreturn(wait_fd_ready(ectx->next_pipe_in));

    errreturn(check_children(children));

    return 0;
}

/// @brief Perform a single iteration for command processing
/// @param cmd_expr Command expression hyper pointer
/// @param cctx Command to fill
/// @return Pointer to the same command
CommandCtx *iter_process_command(CommandExpression ***cmd_expr, CommandCtx *cctx) {
    nullreturn(process_command(cctx, **cmd_expr));
    *cmd_expr += 1;
    return cctx;
}

size_t count_commands(CommandExpression **cmd_expr) {
    size_t count = 0;
    while (*cmd_expr++ != NULL)
        count++;
    return count;
}

int inner_interpret(ExecutionCtx* ectx, CommandCtx **cctxs, CommandExpression **cmd_expr) {
    errreturn(logerr(run_command(ectx, cctxs[0], cctxs[1]), "Failed to run command"));

    while (cctxs[1] != NULL) {
        free_command(cctxs[0]);

        CommandCtx *cmd_temp = cctxs[0];
        cctxs[0] = cctxs[1];
        if (*cmd_expr != NULL) {
            CommandCtx *processed_cmd = iter_process_command(&cmd_expr, cmd_temp);
            if (processed_cmd == NULL) {
                panic("Failed to process command");
            }
            cctxs[1] = processed_cmd;
        } else {
            cctxs[1] = NULL;
        }

        errreturn(logerr(run_command(ectx, cctxs[0], cctxs[1]), "Failed to run command"));
    }
    return 0;
}

/// @brief Interpret a pipe expression
/// @param pipe_expr The pipe expression
/// @param ectx Execution context
void interpret(PipeExpression* pipe_expr, ExecutionCtx* ectx) {
    size_t command_count = count_commands(pipe_expr->Commands);
    children = calloc(command_count + 1, sizeof(int));

    CommandCtx cmd1 = {
        .args = NULL,
        .eenv = NULL,
        .redir_in = NULL,
        .redir_out = NULL,
    };
    CommandCtx cmd2 = {
        .args = NULL,
        .eenv = NULL,
        .redir_in = NULL,
        .redir_out = NULL,
    };
    CommandCtx *cctxs[2] = { NULL, NULL };

    CommandExpression **cmd_expr = pipe_expr->Commands;
    if (*cmd_expr != NULL)
        cctxs[0] = iter_process_command(&cmd_expr, &cmd1);

    if (*cmd_expr != NULL)
        cctxs[1] = iter_process_command(&cmd_expr, &cmd2);

    int res = inner_interpret(ectx, cctxs, cmd_expr);
    if (res < 0) {
        log_info("Execution of commands failed.");
        kill_children(children);
    }

    wait_for_children(children);
    free_command(cctxs[0]);
    free(children);
    children = NULL;
}

void kill_commands() {
    kill_children(children);
}
