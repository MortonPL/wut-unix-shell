#include <glob.h>
#include "cli.h"
#include "../lib/logger.h"
#include "../lib/log.c/src/log.h"

int cd_cmd(const char *file, char* const* argv, char* const* envp) {
    (void)file, (void)envp;
    argv++;
    if (*argv == NULL)
        panic("missing required argument");

    char *new_dir = *argv;

    argv++;
    if (*argv != NULL)
        panic("too many arguments");

    unwrap(chdir(new_dir));
    return 0;
}

#define MAX_PATH 1024

int pwd_cmd(const char *file, char* const* argv, char* const* envp) {
    (void)file, (void)argv, (void)envp;
    char buf[MAX_PATH];

    char *res = getcwd(buf, MAX_PATH);
    if (res != buf)
        panic(OsErrorMessage, strerror(errno));

    printf("%s\n", buf);
    return 0;
}

int echo_cmd(const char *file, char* const* argv, char* const* envp) {
    (void)file, (void)envp;
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
void subprocess_allocate(CommandCtx* cctx, size_t args_count, size_t eenv_count) {
    cctx->args = malloc((args_count + 1) * sizeof(char*));
    (cctx->args)[args_count] = NULL;
    cctx->eenv = malloc((eenv_count + 1) * sizeof(char*));
    (cctx->eenv)[eenv_count] = NULL;
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
    env_iter = environ;
    while (*env_iter != NULL) {
        if (strncmp(key, *env_iter, len) == 0 && (*env_iter)[len] == '=') {
            return *env_iter + len + 1;
        }
        env_iter++;
    }
    return NULL;
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

/// @brief Process a single basic word
/// @param cctx Command context
/// @param word Command word
/// @param returnedCount Count of returned strings
/// @param doGlob Should do glob?
/// @return Processed word
char **process_word(CommandCtx* cctx, CommandWord *word, size_t *returnedCount, bool doGlob) {
    // Find length
    size_t len = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        if ((*els)->Type == WE_VARIABLE_READ) {
            char *env_var = env_get(cctx->eenv, (*els)->Value + 1);
            if (env_var != NULL) {
                size_t var_len = strlen(env_var);
                len += var_len;
            }
        }
        else
            len += (*els)->Length;
        els += 1;
    }

    // Fill
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;
    size_t offset = 0;
    els = word->Elements;
    while (*els != NULL) {
        if ((*els)->Type == WE_VARIABLE_READ) {
            char *env_var = env_get(cctx->eenv, (*els)->Value + 1);
            if (env_var != NULL) {
                size_t var_len = strlen(env_var);
                memcpy(&outbuf[offset], env_var, var_len);
                offset += var_len;
            }
        } else {
            memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
            offset += (*els)->Length;
        }

        els += 1;
    }
    log_trace("");

    char **pReturned = NULL;
    if (doGlob) {
        glob_t globbuf = {};
        glob(outbuf, GLOB_NOMAGIC, NULL, &globbuf);
        (*returnedCount) = globbuf.gl_pathc;
        char **pOriginal = globbuf.gl_pathv;
        pReturned = (char **)malloc(sizeof(char *) * (globbuf.gl_pathc + 1));
        for (size_t i = 0; i < globbuf.gl_pathc; i++) {
            log_trace("Copying glob element: %s", pOriginal[i]);
            pReturned[i] = copyString(pOriginal[i]);
        }
        pReturned[globbuf.gl_pathc] = NULL;
        globfree(&globbuf);
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
void process_command(CommandCtx* cctx, CommandExpression *cmd_expr) {
    size_t args_count = 0, eenv_count = 0;
    subprocess_args_eenv_counts(cmd_expr, &args_count, &eenv_count);
    subprocess_allocate(cctx, args_count, eenv_count);

    size_t args_i = 0, eenv_i = 0;
    CommandWord **words = cmd_expr->Words;
    while (*words != NULL) {
        CommandWord *word = *words++;
        size_t returnedCount = 0;
        char **pArgs = NULL;
        switch (word->Type) {
            case CW_ASSIGNMENT:
                pArgs = process_word(cctx, word, &returnedCount, args_i > 0);
                cctx->eenv[eenv_i++] = pArgs[0];
                free(pArgs);
                break;
            case CW_BASIC: {
                pArgs = process_word(cctx, word, &returnedCount, args_i > 0);
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
                cctx->redir_in = pArgs[0];
                free(pArgs);
                break;
            case CW_REDIRECTION_OUT:
                if (cctx->redir_out != NULL)
                    free(cctx->redir_out);
                pArgs = process_word(cctx, word, &returnedCount, false);
                cctx->redir_out = pArgs[0];
                free(pArgs);
                break;
        }
    }
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
        pipe_in = unwrap(file_in(curr_cctx->redir_in));
        log_trace("Input piped from %s", curr_cctx->redir_in);
    }
    else if (ectx->next_pipe_in == 0) {
        pipe_in = unwrap(file_in((char *)DevNull));
        log_trace("Input piped from %s", DevNull);
    }
    else {
        pipe_in = STDIN_FILENO;
        log_trace("Input piped from standard input");
    }
    
    // Configure out pipe
    if (curr_cctx->redir_out != NULL) {
        pipe_out = unwrap(file_out(curr_cctx->redir_out));
        ectx->next_pipe_in = 0;
        log_trace("Output piped to %s", curr_cctx->redir_out);
    }
    else if (next_cctx == NULL) {
        pipe_out = STDOUT_FILENO;
        ectx->next_pipe_in = -1;
        log_trace("Output piped to standard output");
    }
    else if (next_cctx->redir_in != NULL) {
        pipe_out = unwrap(file_out((char *)DevNull));
        ectx->next_pipe_in = 0;
        log_trace("Output piped to %s", DevNull);
    }
    else {
        create_pipe_pair(&(ectx->next_pipe_in), &pipe_out);
        log_trace("Output piped to next process");
    }
    log_trace("===");

    // Check existence of command
    char *cmd = curr_cctx->args[0];
    if (cmd == NULL) {
        printf("no command provided");
        panic("no command provided");
    }

    // Run commands
    if (strcmp(cmd, "cd") == 0) {
        log_info("Running internal command '%s'", cmd);
        expect(cd_cmd(cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command");
    }
    else if (strcmp(cmd, "echo") == 0) {
        log_info("Running internal command '%s'", cmd);
        expect(attach_command(pipe_in, pipe_out, echo_cmd, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command");
    }
    else if (strcmp(cmd, "pwd") == 0) {
        log_info("Running internal command '%s'", cmd);
        expect(attach_command(pipe_in, pipe_out, pwd_cmd, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run internal command");
    }
    else {
        log_info("Running external command '%s'", cmd);
        expect(attach_command(pipe_in, pipe_out, echo_cmd, cmd, curr_cctx->args, curr_cctx->eenv), "failed to run external command");
    }

    return 0;
}

/// @brief Perform a single iteration for command processing
/// @param cmd_expr Command expression hyper pointer
/// @param cctx Command to fill
/// @return Pointer to the same command
CommandCtx *iter_process_command(CommandExpression ***cmd_expr, CommandCtx *cctx) {
    process_command(cctx, **cmd_expr);
    *cmd_expr += 1;
    return cctx;
}

/// @brief Interpret a pipe expression
/// @param pipe_expr The pipe expression
/// @param ectx Execution context
void interpret(PipeExpression* pipe_expr, ExecutionCtx* ectx) {
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

    run_command(ectx, cctxs[0], cctxs[1]);

    while (cctxs[1] != NULL) {
        free_command(cctxs[0]);

        CommandCtx *cmd_temp = cctxs[0];
        cctxs[0] = cctxs[1];
        cctxs[1] = (*cmd_expr != NULL)
            ? iter_process_command(&cmd_expr, cmd_temp)
            : NULL;

        run_command(ectx, cctxs[0], cctxs[1]);
    }
    free_command(cctxs[0]);
}
