#define _XOPEN_SOURCE 700
#include "shell.h"

void glsh_countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    printf("\033[H\033[J");
    printf("***********************************\n");
    printf("*        Countdown Timer          *\n");
    printf("***********************************\n");
    printf("*          %2d seconds             *\n", i);
    printf("***********************************\n");
    fflush(stdout);
    sleep(1);
  }

  printf("\033[H\033[J");
  printf("***********************************\n");
  printf("*           Time's up!            *\n");
  printf("***********************************\n");
}

void glsh_print_date(void) {
  time_t t = time(NULL);
  if (t == (time_t)-1) {
    perror("glsh: failed to get current time");
    return;
  }

  struct tm tm_info;
  if (localtime_r(&t, &tm_info) == NULL) {
    perror("glsh: failed to convert time");
    return;
  }

  char buffer[64];
  strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Z %Y", &tm_info);
  printf("%s\n", buffer);
}

void glsh_print_time(void) {
  setenv("TZ", "Asia/Ho_Chi_Minh", 1);
  tzset();

  time_t t = time(NULL);
  if (t == (time_t)-1) {
    perror("glsh: failed to get current time");
    return;
  }

  struct tm tm_info;
  if (localtime_r(&t, &tm_info) == NULL) {
    perror("glsh: failed to convert time");
    return;
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
  printf("Current time: %s\n", buffer);
}

void glsh_open_calculator(int is_background) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("glsh: fork");
    return;
  }

  if (pid == 0) {
    setpgid(0, 0);

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    freopen("/dev/null", "w", stderr);
    freopen("/dev/null", "w", stdout);

    char *args[] = {"open", "-W", "-a", "Calculator", NULL};
    execvp("open", args);
    _exit(1);
  } else {
    setpgid(pid, pid);

    if (is_background) {
      glsh_job_add(pid, JOB_RUNNING, "calculator");
    } else {
      tcsetpgrp(STDIN_FILENO, pid);

      int status;
      waitpid(pid, &status, WUNTRACED);

      tcsetpgrp(STDIN_FILENO, getpgrp());

      if (WIFSTOPPED(status)) {
        glsh_job_add(pid, JOB_STOPPED, "calculator");
        printf("\n[%d]+  Stopped                 calculator\n", glsh_job_next_id() - 1);
      }
    }
  }
}

void glsh_open_countdown(int seconds, int is_background) {
  char script[1024];
  snprintf(script, sizeof(script),
    "tell application \"Terminal\"\n"
    "    set newTab to do script \"for i in $(seq %d -1 0); do clear; "
    "echo '***********************************'; "
    "echo '*        Countdown Timer          *'; "
    "echo '***********************************'; "
    "printf '*          %%2d seconds             *\\n' \\\"$i\\\"; "
    "echo '***********************************'; "
    "sleep 1; done; clear; "
    "echo '***********************************'; "
    "echo '*           Time is up!           *'; "
    "echo '***********************************'; "
    "sleep 2\"\n"
    "    activate\n"
    "    repeat\n"
    "        delay 0.5\n"
    "        if not busy of newTab then exit repeat\n"
    "    end repeat\n"
    "end tell",
    seconds);

  pid_t pid = fork();

  if (pid < 0) {
    perror("glsh: fork");
    return;
  }

  if (pid == 0) {
    setpgid(0, 0);

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    freopen("/dev/null", "w", stderr);

    char *args[] = {"osascript", "-e", script, NULL};
    execvp("osascript", args);
    _exit(1);
  } else {
    setpgid(pid, pid);

    char job_name[64];
    snprintf(job_name, sizeof(job_name), "countd %d", seconds);

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
}