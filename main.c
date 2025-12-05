#include "./include/shell.h"
#include "./include/Builtins.h"
#include "./include/introduction.h"
#include "./include/parser.h"
#include "./include/execute.h"

pid_t fg_pid = 0;
int history_count = 0;
char *history[HISTORY_MAX];

void add_to_history(char *line) {
    char *entry = strdup(line);
    entry[strcspn(entry, "\n")] = 0;
    if (history_count >= HISTORY_MAX){
        free(history[history_count % HISTORY_MAX]);
    }
    history[history_count % HISTORY_MAX] = entry;
    history_count++;
}


void lsh_loop(void) {
    Token tokens[1024];
    char *line;
    char **args;
    int status;

    do {
        printf("glsh> ");
        line = lsh_read_line();

        if (strcmp(line, "!!\n") == 0 || strcmp(line, "!!") == 0) {
            if (history_count == 0) {
                printf("lsh: no command in history yet.\n");
                free(line);
                continue;
            }

            char *last_cmd = history[(history_count - 1) % HISTORY_MAX];
            
            printf("%s\n", last_cmd);
            
            free(line); 
            line = strdup(last_cmd); 
        }

        if (line[0] != '\0' && line[0] != '\n') add_to_history(line);

        if (!line || strlen(line) == 0) {
            if (line) free(line);
            continue;
        }
        int n_tok = tokenize(line, tokens);
        if (n_tok > 0) {
            Pipeline *pl = parse(tokens, n_tok);
            
            if (pl->count > 0 && pl->cmds[0]->argc > 0) {
                if (strcmp(pl->cmds[0]->argv[0], "exit") == 0) {
                    break;
                }
                status = execute_pipeline(pl);
            }
            
        }

        free(line);
    }   while (status);
}


int main(int argc, char **args) {
    // Load config files, if any.
    introduction();
    // Run command loop.

    lsh_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}