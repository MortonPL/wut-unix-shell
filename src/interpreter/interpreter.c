#include "cli.h"
#include "../lib/logger.h"
#include "../lib/log.c/src/log.h"

#define COMMAND_SIZE 256
#define FLAG_AMOUNT 2


typedef struct {
    char stack[COMMAND_SIZE][COMMAND_SIZE];
    int redirectIn;
    int redirectOut;
} CommandContext;

/// returns found value in key
void arrayget(char* const* env, char* key) {
    char* variable;
    size_t i = 0;
    while (*env[i] != '\0') {
        if ((variable = strstr(env[i], key)) != NULL) {
            strcpy(key, env[i] + 1 + strlen(key));
            break;
        }
    }
}

unsigned getFlags(char* flags, CommandContext* ctx) {
    unsigned long currentFlag = 0;
    unsigned i = 1;
    while (*(ctx->stack[i]) != '\0' && *(ctx->stack[i]) == '-' && currentFlag < FLAG_AMOUNT) {
        char flag[COMMAND_SIZE];
        strcpy(flag, ctx->stack[i]);
        removeAllOccurences(flag, '-');
        strcpy(&flags[currentFlag], flag);
        currentFlag += strlen(flag);
        i++;
    }
    return i - 1;
}

void changeDirectory(const char* path, ExecutionCtx* env) {
    if (strstr(path, "-") == path) {
        strswp(env->cwd, env->previousWorkingDirectory);
    } else {
        strcpy(env->previousWorkingDirectory, env->cwd);
        if (strstr(path, "..") == path) {
            removeAllAfter(env->cwd, '/');
        } else if (strstr(path, "/") == path) {
            strcpy(env->cwd, path);
        } else {
            strcat(env->cwd, "/");
            strcat(env->cwd, path);
        }
    }
    unwrap(chdir(env->cwd));
}

void pwd_cmd(const char *file, char* const* argv, char* const* envp) {
    char cwd[COMMAND_SIZE] = "cwd";
    arrayget(envp, cwd);
    printf("%s\n", cwd);
}

void echo_cmd(const char *file, char* const* argv, char* const* envp) {
    while (*argv != NULL) {
        printf("%s ", *argv);
        argv++;
    }
    printf("\n");
}

void print(const char* flags, unsigned skip, CommandContext* ctx) {
    if (strstr(flags, "e") != NULL) {
        // TODO implement
    }
    unsigned i = 1 + skip;
    char* element = ctx->stack[i];
    while (strcmp(element, "\0") != 0) {
        printf("%s ", element);
        i++;
        element = ctx->stack[i];
    }
    if (strstr(flags, "n") == NULL) {
        printf("\n");
    }
}

void handleCommand(int argumentCount, CommandContext* ctx, ExecutionCtx* env) {
    // TODO add "export"
    // TODO handle pipes (everywhere) and redirects (in build in)
    char* command = ctx->stack[0];
    if (strcmp(command, "") != 0) {
        if (strstr(command, "cd") == command) {
            if (argumentCount > 1) {
                changeDirectory(ctx->stack[1], env);
            }
        } else if (strstr(command, "pwd") == command) {
            int pid = attach_command(ctx->redirectIn, ctx->redirectOut, pwd_cmd, command, ctx->stack, env->variables);
            if (pid < 0) {
                exit(pid);
            }
            *(env->childPid) = pid;
            int err = wait_for_child(pid);
            if (err < 0) {
                exit(err);
            }
            *(env->childPid) = -1;

            // printWorkingDirectory(env->cwd);
        } else if (strstr(command, "echo") == command) {
            char flags[FLAG_AMOUNT + 1] = "--\0";
            unsigned skip = 0;
            if (argumentCount > 1)
                skip = getFlags(flags, ctx);
            print(flags, skip, ctx);
        } else if (strstr(command, "exit") == command) {
            // handled in interface()
        } else {
            // TODO this forever steals stdout for some reason
            int pid = attach_command(ctx->redirectIn, ctx->redirectOut, NULL, command, ctx->stack, env->variables);
            *(env->childPid) = pid;
            int err = wait_for_child(pid);
            *(env->childPid) = -1;
        }
    }
}

size_t handleWordElement(WordElement* element, char* buffer, ExecutionCtx* env) {
    if (element->Type == WE_BASIC_STRING) {
        strcpy(buffer, element->Value);
        return strlen(element->Value);
    } else if (element->Type == WE_ESCAPED_STRING) {
        // TODO actually escape
        strcpy(buffer, element->Value);
        return strlen(element->Value);
    } else if (element->Type == WE_VARIABLE_READ) {
        char varName[COMMAND_SIZE];
        strcpy(varName, element->Value);
        removeAllOccurences(varName, '$');
        arrayget(env, varName);
        strcpy(buffer, varName);
        return strlen(varName);
    }
    return 0;
}

void handleWordElements(WordElement** elements, size_t length, char* buffer, ExecutionCtx* env) {
    size_t offset = 0;
    for (size_t i=0; i<length; i++) {
        offset += handleWordElement(elements[i], buffer+offset, env);
    }
}

int handleCommandWord(CommandWord* element, int stackIterator, CommandContext* ctx, ExecutionCtx* env) {
    if (element->Type == CW_ASSIGNMENT) {
        if (!(element->Length == 2 && element->Elements[0]->Type == WE_VARIABLE_WRITE)) {
            exit(EXIT_FAILURE);
        }
        char value[256] = {0};
        char temp[256] = {0};
        strcpy(value, element->Elements[0]->Value);
        handleWordElement(element->Elements[1], temp, env);
        strcat(value, temp);
        env->variables[env->variableCount++] = value;
        return 0;
    } else if (element->Type == CW_REDIRECTION_IN) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        ctx->redirectIn = file_in(name);
        return 0;
    } else if (element->Type == CW_REDIRECTION_OUT) {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        ctx->redirectOut = file_out(name);
        return 0;
    } else {
        char name[COMMAND_SIZE] = {0};
        handleWordElements(element->Elements, element->Length, name, env);
        strcpy(ctx->stack[stackIterator], name);
        return 1;
    }
}

void handleCommandExpression(CommandExpression* expression, ExecutionCtx* env) {
    // init context
    CommandContext ctx;
    for (int a=0; a<COMMAND_SIZE; a++) {
        for (int b=0; b<COMMAND_SIZE; b++) {
            ctx.stack[a][b] = 0;
        }
    }
    ctx.redirectIn = STDIN_FILENO;
    ctx.redirectOut = STDOUT_FILENO;
    // actual body
    int argumentCount = 0;
    for(size_t i = 0; i < expression->Length; i++) {
        argumentCount += handleCommandWord(expression->Words[i], argumentCount, &ctx, env);
    }
    handleCommand(argumentCount, &ctx, env);
}

void handlePipeExpression(PipeExpression* expression, ExecutionCtx* env) {
    for (size_t i=0; i < expression->Length; i++) {
        handleCommandExpression(expression->Commands[i], env);
    }
    // TODO actually pipe them
}









/// @brief Free array of strings
/// @param s The array of strings
void free_str_arr(char **s) {
    while (*s != NULL) {
        log_trace("ww3 %s", *s);
        free(*s++);
        log_trace("ww3");
    }
    log_trace("");
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
                eenv_count++;
                break;
            case CW_BASIC:
                args_count++;
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
    log_trace("%i", args_count);
    cctx->args = malloc(args_count + 1);
    log_trace("%i", cctx->args);
    (cctx->args)[args_count] = NULL;
    log_trace("");
    cctx->eenv = malloc(eenv_count +1);
    log_trace("");
    (cctx->eenv)[eenv_count] = NULL;
    log_trace("");
}

/// @brief Process a single assignment word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_assignment(CommandCtx* cctx, CommandWord *word) {
    size_t len = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        len += (*els)->Length;
        els += 1;
    }
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;

    size_t offset = 0;
    els = word->Elements;
    while (*els != NULL) {
        memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
        offset += (*els)->Length;
        els += 1;
    }

    return outbuf;
}

char* env_get(char **env, char *key) {
    char **env_iter = env;
    log_trace("%i", env);
    while (*env_iter != NULL) {
        log_trace("env iter");
        if (strncmp(key, env_iter, strlen(key)) == 0)
            return env_iter;
        env_iter++;
    }
    log_trace("");
    env_iter = environ;
    while (*env_iter != NULL) {
        log_trace("environ iter");
        if (strncmp(key, env_iter, strlen(key)) == 0)
            return env_iter;
        env_iter++;
    }
    log_trace("");
    return "";
}

/// @brief Process a single basic word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_basic(CommandCtx* cctx, CommandWord *word) {
    log_trace("");
    size_t len = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        log_trace("ww1");
        if ((*els)->Type == WE_VARIABLE_READ)
            len += (*els)->Length - 1;
        else
            len += (*els)->Length;
        els += 1;
    }
    log_trace("");
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;
    log_trace("");
    size_t offset = 0;
    els = word->Elements;
    log_trace("");
    while (*els != NULL) {
        log_trace("ww2");
        if ((*els)->Type == WE_VARIABLE_READ) {
            log_trace("");
            char *env_var = env_get(cctx->eenv, (*els)->Value + 1);
            log_trace("");
            size_t var_len = strlen(env_var);
            log_trace("");
            memcpy(&outbuf[offset], env_var, var_len);
            log_trace("");
            offset += var_len;
        }
        else {
            log_trace("");
            memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
            offset += (*els)->Length;
        }
        
        els += 1;
    }
    log_trace("");

    return outbuf;
}

/// @brief Process a single redirect in word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_redir_in(CommandCtx* cctx, CommandWord *word) {
    size_t len = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        len += (*els)->Length;
        els += 1;
    }
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;

    size_t offset = 0;
    els = word->Elements;
    while (*els != NULL) {
        memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
        offset += (*els)->Length;
        els += 1;
    }

    return outbuf;
}

/// @brief Process a single redirect out word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_redir_out(CommandCtx* cctx, CommandWord *word) {
    size_t len = 0;
    WordElement **els = word->Elements;
    while (*els != NULL) {
        len += (*els)->Length;
        els += 1;
    }
    char *outbuf = malloc(len + 1);
    outbuf[len] = 0;

    size_t offset = 0;
    els = word->Elements;
    while (*els != NULL) {
        memcpy(&outbuf[offset], (*els)->Value, (*els)->Length);
        offset += (*els)->Length;
        els += 1;
    }

    return outbuf;
}

/// @brief Fill command ctx with information from command expression
/// @param cctx Context to fill
/// @param cmd_expr Command expression to consume
void process_command(CommandCtx* cctx, CommandExpression *cmd_expr) {
    size_t args_count = 0, eenv_count = 0;
    log_trace("");
    subprocess_args_eenv_counts(cmd_expr, &args_count, &eenv_count);
    log_trace("");
    subprocess_allocate(cctx, args_count, eenv_count);
    log_trace("");

    size_t args_i = 0, eenv_i = 0;
    CommandWord **words = cmd_expr->Words;
    while (*words != NULL) {
        log_trace("w1");
        CommandWord *word = *words++;
        log_trace("w2");
        switch (word->Type) {
            case CW_ASSIGNMENT:
                cctx->eenv[eenv_i++] = process_word_assignment(cctx, word);
                break;
            case CW_BASIC:
                cctx->args[args_i++] = process_word_basic(cctx, word);
                args_i++;
                break;
            case CW_REDIRECTION_IN:
                if (cctx->redir_in != NULL) free(cctx->redir_in);
                cctx->redir_in = process_word_redir_in(cctx, word);
                break;
            case CW_REDIRECTION_OUT:
                if (cctx->redir_out != NULL) free(cctx->redir_out);
                cctx->redir_out = process_word_redir_out(cctx, word);
                break;
        }
        log_trace("w3");
        DeleteCommandWord(word);
    }
    free(cmd_expr);
}

/// @brief Free command ctx struct
/// @param cctx Pointer to the struct
void free_command(CommandCtx* cctx) {
    log_trace("");
    if (cctx == NULL) return;
    log_trace("");
    if (cctx->args != NULL) {
        free_str_arr(cctx->args);
        cctx->args = NULL;
    }
    log_trace("");
    if (cctx->eenv != NULL) {
        free_str_arr(cctx->eenv);
        cctx->eenv = NULL;
    }
    log_trace("");
    if (cctx->redir_in != NULL) {
        free(cctx->redir_in);
        cctx->redir_in = NULL;
    }
    log_trace("");
    if (cctx->redir_out != NULL) {
        free(cctx->redir_out);
        cctx->redir_out = NULL;
    }
    log_trace("");
}

int run_command(ExecutionCtx* ectx, CommandCtx* curr_cctx, CommandCtx* next_cctx) {
    log_trace("");
    attach_command(STDIN_FILENO, STDOUT_FILENO, echo_cmd, "", curr_cctx->args, curr_cctx->eenv);
    log_trace("");
}

/// @brief Perform a single iteration for command processing
/// @param cmd_expr Command expression hyper pointer
/// @param cctx Command to fill
/// @return Pointer to the same command
CommandCtx *iter_process_command(CommandExpression ***cmd_expr, CommandCtx *cctx) {
    log_trace("");
    process_command(cctx, **cmd_expr);
    log_trace("");
    **cmd_expr = NULL;
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

    log_trace("");
    CommandExpression **cmd_expr = pipe_expr->Commands;
    if (*cmd_expr != NULL)
        cctxs[0] = iter_process_command(&cmd_expr, &cmd1);

    log_trace("");
    if (*cmd_expr != NULL)
        cctxs[1] = iter_process_command(&cmd_expr, &cmd2);

    log_trace("");
    run_command(ectx, cctxs[0], cctxs[1]);

    while (cctxs[1] != NULL) {
        free_command(cctxs[0]);

        int cmd_temp = cctxs[0];
        cctxs[0] = cctxs[1];
        cctxs[1] = (*cmd_expr != NULL)
            ? iter_process_command(&cmd_expr, cmd_temp)
            : NULL;

        run_command(ectx, cctxs[0], cctxs[1]);
    }

    free_command(cctxs[0]);
}
