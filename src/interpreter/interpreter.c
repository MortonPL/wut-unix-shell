#include "cli.h"

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
    if (chdir(env->cwd) < 0) {
        exit(EXIT_FAILURE);
    }
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









/// @brief Counts amount of args and eenv in a command
/// @param cExpression Command expression to count from
/// @param args_count Pointer to args count
/// @param eenv_count Pointer to eenv count
void subprocess_args_eenv_counts(CommandExpression *cmd_expr, size_t *args_count, size_t *eenv_count) {
    CommandWord **words = cmd_expr->Words;
    while (*words != NULL) {
        CommandWord *word = *words++;
        switch (word->Type) {
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

/// @brief Count amount of entries in environ
/// @param environ_count Pointer to the environ count
void subprocess_environ_count(size_t *environ_count) {
    char **evar = environ;
    while (*evar++ != NULL)
        environ_count++;
}

/// @brief Allocate space for command context, copy environ
/// @param ctx Command context
/// @param args_count Args count
/// @param eenv_count Eenv count
/// @param environ_count Environ count
void subprocess_allocate(CommandCtx* cctx, size_t args_count, size_t eenv_count, size_t environ_count) {
    cctx->args = malloc(args_count + 1);
    cctx->args[args_count] = NULL;
    cctx->eenv = malloc(eenv_count + environ_count + 1);
    cctx->eenv[eenv_count + environ_count] = NULL;
    memcpy(cctx->eenv + eenv_count, environ, environ_count * sizeof(char*));
}

/// @brief Process a single word element
/// @param cctx Command context
/// @param el Word element
/// @return Processed word element
char* process_word_element(CommandCtx* cctx, WordElement *el) {
    
}

/// @brief Process a single assignment word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_assignment(CommandCtx* cctx, CommandWord *word) {
    return "";
}

/// @brief Process a single basic word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_basic(CommandCtx* cctx, CommandWord *word) {

}

/// @brief Process a single redirect in word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_redir_in(CommandCtx* cctx, CommandWord *word) {
    return "";
}

/// @brief Process a single redirect out word
/// @param cctx Command context
/// @param word Command word
/// @return Processed word
char* process_word_redir_out(CommandCtx* cctx, CommandWord *word) {
    return "";
}

/// @brief Fill command ctx with information from command expression
/// @param ctx Context to fill
/// @param cExpression Command expression to consume
void process_command(CommandCtx* cctx, CommandExpression *cmd_expr) {
    size_t args_count, eenv_count, environ_count;
    subprocess_args_eenv_counts(cmd_expr, &args_count, &eenv_count);
    subprocess_environ_count(&environ_count);
    subprocess_allocate(cctx, args_count, eenv_count, environ_count);

    size_t args_i, eenv_i;
    CommandWord **words = cmd_expr->Words;
    while (*words != NULL) {
        CommandWord *word = *words++;
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
    }
}

/// @brief Free array of strings
/// @param s The array of strings
void free_str_arr(char **s) {
    while (*s != NULL)
        free(*s++);
}

/// @brief Free command ctx struct
/// @param ctx Pointer to the struct
void free_command(CommandCtx* cctx) {
    if (cctx == NULL) return;
    if (cctx->args != NULL) free_str_arr(cctx->args);
    if (cctx->eenv != NULL) free_str_arr(cctx->eenv);
    if (cctx->redir_in != NULL) free(cctx->redir_in);
    if (cctx->redir_out != NULL) free(cctx->redir_out);
    free(cctx);
}

int run_command(ExecutionCtx* ectx, CommandCtx* curr_cctx, CommandCtx* next_cctx) {

}

/// @brief Perform a single iteration for command processing
/// @param i_ptr Command expression hyper pointer
/// @param cmd Command to fill
/// @return Pointer to the same command
CommandCtx *iter_process_command(CommandExpression ***cmd_expr, CommandCtx *cctx) {
    process_command(cctx, **cmd_expr);
    **cmd_expr = NULL;
    *cmd_expr += 1;
    return cctx;
}

/// @brief Interpret a pipe expression
/// @param pExpression The pipe expression
/// @param env Execution context
void interpret(PipeExpression* pipe_expr, ExecutionCtx* ectx) {
    CommandCtx cmd1, cmd2;
    CommandCtx *cctxs[2] = { NULL, NULL };

    CommandExpression **cmd_expr = pipe_expr->Commands;
    if (*cmd_expr != NULL)
        cctxs[0] = iter_process_command(&cmd_expr, &cmd1);

    if (*cmd_expr != NULL)
        cctxs[1] = iter_process_command(&cmd_expr, &cmd2);

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
