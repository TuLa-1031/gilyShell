#include "shell.h"
#include "Builtins.h"
#include "utils.h"

/*
    Function Declarations for builtin shell commands;
*/
int lsh_cd(char **args);
int help(char **args);
int lsh_exit(char **args);
int lsh_history(char **args);
int lsh_countd(char **args);
int lsh_date(char **args);
int lsh_time(char **args);


/*
    List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "history",
    "countd",
    "date",
    "time"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &help,
    &lsh_exit,
    &lsh_history,
    &lsh_countd,
    &lsh_date,
    &lsh_time
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
 /*
    Builtin function implementations.
 */
 int lsh_cd(char **args) {
    if (args[1]==NULL) {
        fprintf(stderr, "lsh: expeccted argument to \"cd\"\n");
    }   else{
        if (chdir(args[1])!=0){
            perror("lsh");
        }
    }
    return 1;
 }


int help(char **args) {
    int i;
    printf("Le Tung Lam's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in::\n");

    for (i=0;i<lsh_num_builtins();i++){
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other program.\n");
    return 1;
 }

 int lsh_exit(char **args){
    return 0;
 }

 int lsh_history(char **args){
    int start_index = (history_count > HISTORY_MAX) ? (history_count - HISTORY_MAX) : 0;
    for (int i = start_index; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i % HISTORY_MAX]);
    }
    return 1;
 }

 int lsh_countd(char **args) {
    int i = atoi(args[1]);
    countdown(i);
    return 1;
 }

 int lsh_date(char **args) {
    date();
    return 1;
 }

 int lsh_time(char **args) {
    time_();
    return 1;
 }