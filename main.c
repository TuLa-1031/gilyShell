#include "shell.h"
#include "Builtins.h"
#include "introduction.h"
#include "parser.h"
#include "launch.h"

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


int lsh_execute(char **args){
    int i;

    if (args[0]==NULL){
        // AN empty command was entered.
        return 1;
    }

    for (i=0;i<lsh_num_builtins();i++){
        if (strcmp(args[0], builtin_str[i])==0){
            return (*builtin_func[i])(args);
        }
    }

    //printf("lsh: No such file or directory\n");
    //free(history[history_count]);
    //history_count--;
    //if (history_count < 0) history_count = HISTORY_MAX;
    //return 1;
    return lsh_launch(args);
}


void lsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
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

        args = lsh_split_line(line);
        args = expand_to_glob_argv(args);
        status = lsh_execute(args);

        free(line);
        free(args);
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