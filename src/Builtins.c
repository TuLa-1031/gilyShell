#include "Builtins.h"
#include "shell.h"
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
int lsh_calculator(char **args);

int lsh_env(char **args);
int lsh_print_path(char **args);

/*
    List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {"cd",     "help", "exit", "history",
                       "countd", "date", "time", "calculator", "showEnv",
                       "printPath"};

char *builtin_desc[] = {"Change the current working directory.",
                        "Display information about builtin commands.",
                        "Terminate the shell.",
                        "Display the history of commands.",
                        "Count down timer",
                        "Display the current date.",
                        "Display the current time.",
                        "Perform basic arithmetic operations."};

char *builtin_usage[] = {"cd <directory>", "help [command]", "exit",
                         "history",        "countd <num>",   "date",
                         "time",           "calculator <op>"};

int (*builtin_func[])(char **) = {&lsh_cd,      &help,          &lsh_exit,
                                  &lsh_history, &lsh_countd,    &lsh_date,
                                  &lsh_time,    &lsh_calculator,&lsh_env,
                                  &lsh_print_path};

int lsh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }
/*
   Builtin function implementations.
*/
int lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: expeccted argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int help(char **args) {
  int i;
  printf("--- Le Tung Lam's LSH ---\n");

  if (args[1] == NULL) {
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n\n");

    printf("  %-12s %-30s %s\n", "COMMAND", "USAGE", "DESCRIPTION");
    printf("  %-12s %-30s %s\n", "-------", "-----", "-----------");

    for (i = 0; i < lsh_num_builtins(); i++) {
      printf("  %-12s %-30s %s\n", builtin_str[i], builtin_usage[i],
             builtin_desc[i]);
    }

    printf("\nUse 'help <command>' for specific details.\n");
  } else {
    int found = 0;
    for (i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(args[1], builtin_str[i]) == 0) {
        printf("Info for built-in command: '%s'\n", args[1]);
        printf("  Usage:       %s\n", builtin_usage[i]);
        printf("  Description: %s\n", builtin_desc[i]);
        found = 1;
        break;
      }
    }

    if (!found) {
      printf("LSH: No help entry for '%s'.\n", args[1]);
    }
  }
  return 1;
}

int lsh_exit(char **args) { return 0; }

int lsh_history(char **args) {
  int start_index =
      (history_count > HISTORY_MAX) ? (history_count - HISTORY_MAX) : 0;
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

int lsh_calculator(char **args) {
  openCalculator(0);
  return 1;
}

int lsh_env(char **args) {
  printf("ltl hihi");
  for (char **e = environ; *e != NULL; e++) {
    printf("%s\n", *e);
  }
  return 1;
}

int lsh_print_path(char **args) {
    char *path = getenv("PATH");
    if (path != NULL) {
        printf("PATH=%s\n", path);
    } else {
        printf("PATH variable not found.\n");
    }
    return 1;
}