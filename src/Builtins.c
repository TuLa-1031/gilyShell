#include "Builtins.h"
#include "shell.h"
#include "utils.h"
#include <dirent.h>

static int builtin_cd(char **args);
static int builtin_help(char **args);
static int builtin_exit(char **args);
static int builtin_history(char **args);
static int builtin_countd(char **args);
static int builtin_date(char **args);
static int builtin_time(char **args);
static int builtin_calculator(char **args);
static int builtin_env(char **args);
static int builtin_path(char **args);
static int builtin_jobs(char **args);
static int builtin_fg(char **args);
static int builtin_bg(char **args);
static int builtin_repeat(char **args);
static int builtin_setenv(char **args);
static int builtin_unsetenv(char **args);
static int builtin_getenv(char **args);
static int builtin_env_cmd(char **args);
static int builtin_toggle_exec(char **args);
static int builtin_clear(char **args);
static int builtin_ls(char **args);

char *glsh_builtin_names[] = {
    "cd",    "help",       "exit",   "history",  "countd",   "date",
    "time",  "calculator", "showEnv", "printPath", "jobs",    "fg",
    "bg",    "repeat",     "setenv", "unsetenv", "getenv",  "env",
    "exec", "clear",      "ls"
};

static char *builtin_desc[] = {
    "Navigate to a specified directory.",
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
    "Resume stopped job in background.",
    "Repeat a command N times.",
    "Set an environment variable.",
    "Remove an environment variable.",
    "Get value of an environment variable.",
    "Run command with modified environment or print environment.",
    "Toggle external command execution on/off.",
    "Clear the terminal screen.",
    "List directory contents."
};

static char *builtin_usage[] = {
    "cd <directory>",    "help [command]",     "exit",
    "history",           "countd <seconds>",   "date",
    "time",              "calculator",         "showEnv",
    "printPath",         "jobs",               "fg <job_id>",
    "bg <job_id>",       "repeat <n> <cmd>",   "setenv <name> <value>",
    "unsetenv <name>",   "getenv <name>",      "env [NAME=VALUE]... [cmd]",
    "exec [on|off]",     "clear",              "ls [path]"
};

int (*glsh_builtin_funcs[])(char **) = {
    &builtin_cd,       &builtin_help,    &builtin_exit,
    &builtin_history,  &builtin_countd,  &builtin_date,
    &builtin_time,     &builtin_calculator, &builtin_env,
    &builtin_path,     &builtin_jobs,    &builtin_fg,
    &builtin_bg,       &builtin_repeat,  &builtin_setenv,
    &builtin_unsetenv, &builtin_getenv,  &builtin_env_cmd,
    &builtin_toggle_exec, &builtin_clear, &builtin_ls
};

int glsh_num_builtins(void) {
  return sizeof(glsh_builtin_names) / sizeof(char *);
}

static int builtin_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: expected argument to \"cd\"\n");
    return 1;
  }

  if (chdir(args[1]) != 0) {
    perror("glsh");
  }
  return 1;
}

static int builtin_help(char **args) {
  if (args[1] == NULL) {
    printf("\n");
    printf("  GLSH - Gily Shell\n");
    printf("  ==================\n");
    printf("\n");
    printf("  NAVIGATION\n");
    printf("    cd <dir>          Change directory\n");
    printf("    ls [path]         List directory contents\n");
    printf("    clear             Clear terminal screen\n");
    printf("\n");
    printf("  ENVIRONMENT\n");
    printf("    showEnv           Show all environment variables\n");
    printf("    printPath         Print PATH variable\n");
    printf("    setenv <n> <v>    Set environment variable\n");
    printf("    unsetenv <name>   Remove environment variable\n");
    printf("    getenv <name>     Get environment variable value\n");
    printf("    env [cmd]         Run with modified environment\n");
    printf("\n");
    printf("  JOB CONTROL\n");
    printf("    jobs              List background jobs\n");
    printf("    fg <id>           Bring job to foreground\n");
    printf("    bg <id>           Resume job in background\n");
    printf("\n");
    printf("  UTILITIES\n");
    printf("    date              Show current date\n");
    printf("    time              Show current time\n");
    printf("    countd <sec>      Countdown timer\n");
    printf("    calculator        Open calculator app\n");
    printf("    repeat <n> <cmd>  Repeat command N times\n");
    printf("    history           Show command history\n");
    printf("\n");
    printf("  SHELL\n");
    printf("    exec [on|off]     Toggle external command execution\n");
    printf("    help [cmd]        Show help\n");
    printf("    exit              Exit the shell\n");
    printf("\n");
    printf("  Tip: Use '!!' to repeat last command, '&' for background jobs\n");
    printf("\n");
    return 1;
  }

  for (int i = 0; i < glsh_num_builtins(); i++) {
    if (strcmp(args[1], glsh_builtin_names[i]) == 0) {
      printf("\n");
      printf("  Command:     %s\n", glsh_builtin_names[i]);
      printf("  Usage:       %s\n", builtin_usage[i]);
      printf("  Description: %s\n", builtin_desc[i]);
      printf("\n");
      return 1;
    }
  }

  printf("glsh: no help available for '%s'\n", args[1]);
  return 1;
}

static int builtin_exit(char **args) {
  (void)args;
  return 0;
}

static int builtin_history(char **args) {
  (void)args;
  int start = (glsh_history_count > GLSH_HISTORY_MAX)
                  ? (glsh_history_count - GLSH_HISTORY_MAX)
                  : 0;

  for (int i = start; i < glsh_history_count; i++) {
    printf("%d: %s\n", i + 1, glsh_history[i % GLSH_HISTORY_MAX]);
  }
  return 1;
}

static int builtin_countd(char **args) {
  return glsh_builtin_countd(args, 0);
}

int glsh_builtin_countd(char **args, int is_background) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: expected argument to \"countd\"\n");
    return 1;
  }

  int seconds = atoi(args[1]);
  glsh_open_countdown(seconds, is_background);
  return 1;
}

static int builtin_date(char **args) {
  (void)args;
  glsh_print_date();
  return 1;
}

static int builtin_time(char **args) {
  (void)args;
  glsh_print_time();
  return 1;
}

static int builtin_calculator(char **args) {
  (void)args;
  glsh_open_calculator(0);
  return 1;
}

int glsh_builtin_calculator(char **args, int is_background) {
  (void)args;
  glsh_open_calculator(is_background);
  return 1;
}

static int builtin_env(char **args) {
  (void)args;
  for (char **env = environ; *env != NULL; env++) {
    printf("%s\n", *env);
  }
  return 1;
}

static int builtin_path(char **args) {
  (void)args;
  char *path = getenv("PATH");

  if (path != NULL) {
    printf("PATH=%s\n", path);
  } else {
    printf("PATH variable not set.\n");
  }
  return 1;
}

static int builtin_setenv(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    fprintf(stderr, "glsh: usage: setenv <name> <value>\n");
    return 1;
  }

  if (setenv(args[1], args[2], 1) != 0) {
    perror("glsh: setenv");
  } else {
    printf("%s=%s\n", args[1], args[2]);
  }
  return 1;
}

static int builtin_unsetenv(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: usage: unsetenv <name>\n");
    return 1;
  }

  if (unsetenv(args[1]) != 0) {
    perror("glsh: unsetenv");
  } else {
    printf("Unset: %s\n", args[1]);
  }
  return 1;
}

static int builtin_getenv(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: usage: getenv <name>\n");
    return 1;
  }

  char *value = getenv(args[1]);
  if (value != NULL) {
    printf("%s=%s\n", args[1], value);
  } else {
    printf("%s is not set\n", args[1]);
  }
  return 1;
}

static int builtin_env_cmd(char **args) {
  int i = 1;

  while (args[i] && strchr(args[i], '=')) {
    char *var = strdup(args[i]);
    char *eq = strchr(var, '=');
    *eq = '\0';
    setenv(var, eq + 1, 1);
    free(var);
    i++;
  }

  if (args[i] == NULL) {
    for (char **env = environ; *env != NULL; env++) {
      printf("%s\n", *env);
    }
  } else {
    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[i], &args[i]);
      fprintf(stderr, "glsh: %s: command not found\n", args[i]);
      _exit(127);
    } else if (pid > 0) {
      int status;
      waitpid(pid, &status, 0);
    } else {
      perror("glsh: fork");
    }
  }
  return 1;
}

static int builtin_toggle_exec(char **args) {
  if (args[1] == NULL) {
    printf("External command execution: %s\n", glsh_exec_enabled ? "ON" : "OFF");
  } else if (strcmp(args[1], "on") == 0) {
    glsh_exec_enabled = 1;
    printf("External command execution: ON\n");
  } else if (strcmp(args[1], "off") == 0) {
    glsh_exec_enabled = 0;
    printf("External command execution: OFF\n");
  } else {
    fprintf(stderr, "glsh: usage: exec [on|off]\n");
  }
  return 1;
}

static int builtin_clear(char **args) {
  (void)args;
  printf("\033[H\033[J");
  fflush(stdout);
  return 1;
}

static int builtin_ls(char **args) {
  const char *path = args[1] ? args[1] : ".";
  DIR *dir = opendir(path);
  
  if (!dir) {
    fprintf(stderr, "glsh: ls: %s: ", path);
    perror("");
    return 1;
  }
  
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] != '.') {
      printf("%s  ", entry->d_name);
    }
  }
  printf("\n");
  closedir(dir);
  return 1;
}

static int builtin_jobs(char **args) {
  (void)args;
  glsh_jobs_print();
  return 1;
}

static int builtin_fg(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: usage: fg <job_id>\n");
    return 1;
  }

  int id = atoi(args[1]);
  Job *job = glsh_job_find_by_id(id);
  if (!job) {
    fprintf(stderr, "glsh: job not found: %s\n", args[1]);
    return 1;
  }

  int continue_job = (job->status == JOB_STOPPED);
  job->status = JOB_RUNNING;
  glsh_jobs_print();

  if (tcsetpgrp(STDIN_FILENO, job->pid) == -1) {
    perror("glsh: tcsetpgrp");
  }

  if (continue_job) {
    kill(-job->pid, SIGCONT);
  }

  int status;
  waitpid(job->pid, &status, WUNTRACED);

  tcsetpgrp(STDIN_FILENO, getpgrp());

  if (WIFSTOPPED(status)) {
    job->status = JOB_STOPPED;
    printf("\n[%d]+  Stopped                 %s\n", job->id, job->command);
  } else {
    glsh_job_delete(job->pid);
  }

  return 1;
}

static int builtin_bg(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "glsh: usage: bg <job_id>\n");
    return 1;
  }

  int id = atoi(args[1]);
  Job *job = glsh_job_find_by_id(id);
  if (!job) {
    fprintf(stderr, "glsh: job not found: %s\n", args[1]);
    return 1;
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

static int builtin_repeat(char **args) {
  return glsh_builtin_repeat(args, 0);
}

int glsh_builtin_repeat(char **args, int is_background) {
  if (args[1] == NULL || args[2] == NULL) {
    fprintf(stderr, "glsh: usage: repeat <n> <command> [args...]\n");
    return 1;
  }

  int count = atoi(args[1]);
  if (count <= 0) {
    fprintf(stderr, "glsh: repeat count must be positive\n");
    return 1;
  }

  char job_name[256] = "repeat ";
  for (int i = 1; args[i]; i++) {
    strncat(job_name, args[i], sizeof(job_name) - strlen(job_name) - 2);
    if (args[i + 1]) {
      strncat(job_name, " ", sizeof(job_name) - strlen(job_name) - 1);
    }
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("glsh: fork");
    return 1;
  }

  if (pid == 0) {
    setpgid(0, 0);

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    if (is_background) {
      freopen("/dev/null", "r", stdin);
    } else {
      tcsetpgrp(STDIN_FILENO, getpid());
    }

    for (int i = 0; i < count; i++) {
      pid_t child = fork();
      if (child == 0) {
        execvp(args[2], &args[2]);
        perror("glsh");
        _exit(127);
      } else if (child > 0) {
        int status;
        waitpid(child, &status, 0);
      }
    }
    _exit(0);
  } else {
    setpgid(pid, pid);

    if (is_background) {
      glsh_job_add(pid, JOB_RUNNING, job_name);
    } else {
      tcsetpgrp(STDIN_FILENO, pid);

      int status;
      waitpid(pid, &status, WUNTRACED);

      tcsetpgrp(STDIN_FILENO, getpgrp());

      if (WIFSTOPPED(status)) {
        glsh_job_add(pid, JOB_STOPPED, job_name);
        printf("\n[%d]+  Stopped                 %s\n", glsh_job_next_id() - 1, job_name);
      }
    }
  }

  return 1;
}