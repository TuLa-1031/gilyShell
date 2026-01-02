#include "Builtins.h"
#include "shell.h"
#include "utils.h"

/*
  Function Declarations for builtin shell commands
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_history(char **args);
int lsh_countd(char **args);
int lsh_date(char **args);
int lsh_time(char **args);
int lsh_calculator(char **args);
int lsh_env(char **args);
int lsh_print_path(char **args);
int lsh_jobs(char **args);
int lsh_fg(char **args);
int lsh_bg(char **args);

/*
  Builtin command definitions
*/
char *builtin_str[] = {"cd",   "help", "exit",       "history", "countd",
                       "date", "time", "calculator", "showEnv", "printPath",
                       "jobs", "fg",   "bg"};

char *builtin_desc[] = {"Navigate to a specified directory.",
                        "Show help information for available commands.",
                        "Exit the shell session.",
                        "List previously executed commands.",
                        "Start a countdown timer from a given number.",
                        "Print the current system date.",
                        "Print the current system time.",
                        "Open an interactive arithmetic calculator.",
                        "List all environment variables.",
                        "Print the current PATH environment variable.",
                        "List active jobs.",
                        "Move background job to foreground.",
                        "Resume stopped job in background."};

char *builtin_usage[] = {
    "cd <directory>",   "help [command]", "exit", "history",
    "countd <seconds>", "date",           "time", "calculator",
    "showEnv",          "printPath",      "jobs", "fg <job_id>",
    "bg <job_id>"};

int (*builtin_func[])(char **) = {
    &lsh_cd,   &lsh_help, &lsh_exit,       &lsh_history, &lsh_countd,
    &lsh_date, &lsh_time, &lsh_calculator, &lsh_env,     &lsh_print_path,
    &lsh_jobs, &lsh_fg,   &lsh_bg};

int lsh_num_builtins(void) { return sizeof(builtin_str) / sizeof(char *); }

/*
   Builtin function implementations
*/
int lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: expected argument to \"cd\"\n");
    return 1;
  }

  if (chdir(args[1]) != 0) {
    perror("glsh");
  }
  return 1;
}

int lsh_help(char **args) {
  printf("=== GLSH - Gily Shell ===\n\n");

  if (args[1] == NULL) {
    printf("Enter commands followed by arguments and press Enter.\n");
    printf("Available built-in commands:\n\n");
    printf("  %-12s %-20s %s\n", "Command", "Usage", "Description");
    printf("  %-12s %-20s %s\n", "-------", "-----", "-----------");

    for (int i = 0; i < lsh_num_builtins(); i++) {
      printf("  %-12s %-20s %s\n", builtin_str[i], builtin_usage[i],
             builtin_desc[i]);
    }

    printf("\nType 'help <command>' for detailed information.\n");
    return 1;
  }

  for (int i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[1], builtin_str[i]) == 0) {
      printf("Command: %s\n", builtin_str[i]);
      printf("  Usage:       %s\n", builtin_usage[i]);
      printf("  Description: %s\n", builtin_desc[i]);
      return 1;
    }
  }

  printf("glsh: no help available for '%s'\n", args[1]);
  return 1;
}

int lsh_exit(char **args) {
  (void)args;
  return 0;
}

int lsh_history(char **args) {
  (void)args;
  int start = (history_count > HISTORY_MAX) ? (history_count - HISTORY_MAX) : 0;

  for (int i = start; i < history_count; i++) {
    printf("%d: %s\n", i + 1, history[i % HISTORY_MAX]);
  }
  return 1;
}

int lsh_countd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: expected argument to \"countd\"\n");
    return 1;
  }

  int seconds = atoi(args[1]);
  countdown(seconds);
  return 1;
}

int lsh_date(char **args) {
  (void)args;
  date();
  return 1;
}

int lsh_time(char **args) {
  (void)args;
  time_();
  return 1;
}

int lsh_calculator(char **args) {
  (void)args;
  openCalculator(0);
  return 1;
}

int lsh_env(char **args) {
  (void)args;
  for (char **env = environ; *env != NULL; env++) {
    printf("%s\n", *env);
  }
  return 1;
}

int lsh_print_path(char **args) {
  (void)args;
  char *path = getenv("PATH");

  if (path != NULL) {
    printf("PATH=%s\n", path);
  } else {
    printf("PATH variable not set.\n");
  }
  return 1;
}

int lsh_jobs(char **args) {
  (void)args;
  print_jobs();
  return 1;
}

int lsh_fg(char **args) {
  Job *job;
  if (args[1] == NULL) {
    job = NULL;
    fprintf(stderr, "glsh: usage: fg <job_id>\n");
    return 1;
  } else {
    int id = atoi(args[1]);
    job = find_job_by_id(id);
    if (!job) {
      fprintf(stderr, "glsh: job not found: %s\n", args[1]);
      return 1;
    }
  }

  job->status = JOB_RUNNING;
  print_jobs();

  if (job->status == JOB_RUNNING) {
    
  }

  // Give terminal control to the job
  if (tcsetpgrp(STDIN_FILENO, job->pid) == -1) {
    perror("tcsetpgrp");
  }

  // Continue process if stopped
  if (job->status == JOB_STOPPED) {
    kill(-job->pid, SIGCONT); 
  }

  job->status = JOB_RUNNING;

  // Wait for it
  int status;
  waitpid(job->pid, &status, WUNTRACED);

  // Restore terminal control to shell
  tcsetpgrp(STDIN_FILENO, getpgrp());

  if (WIFSTOPPED(status)) {
    job->status = JOB_STOPPED;
    printf("\n[%d]+  Stopped                 %s\n", job->id, job->command);
  } else {
    delete_job(job->pid);
  }

  return 1;
}

int lsh_bg(char **args) {
  Job *job;
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: usage: bg <job_id>\n");
    return 1;
  } else {
    int id = atoi(args[1]);
    job = find_job_by_id(id);
    if (!job) {
      fprintf(stderr, "glsh: job not found: %s\n", args[1]);
      return 1;
    }
  }

  if (job->status == JOB_RUNNING) {
    fprintf(stderr, "glsh: job %d is already running\n", job->id);
    return 1;
  }

  job->status = JOB_RUNNING;
  printf("[%d]+ %s &\n", job->id, job->command);
  kill(job->pid, SIGCONT);

  return 1;
}