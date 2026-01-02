#define _XOPEN_SOURCE 700
#include "shell.h"
#include <termios.h>

// ... existing code ...

static void handle_parent_foreground(pid_t pid) {
  fg_pid = pid;
  strcpy(fg_command_name, "Calculator");

  int status = 0;
  waitpid(pid, &status, WUNTRACED);

  // Restore terminal control to shell
  tcsetpgrp(STDIN_FILENO, getpgrp());

  // Flush any input typed while the command was running
  tcflush(STDIN_FILENO, TCIFLUSH);

  fg_pid = -1;
}

void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    system("clear");
    printf("***********************************\n");
    printf("*        Countdown Timer          *\n");
    printf("***********************************\n");
    printf("*          %2d seconds             *\n", i);
    printf("***********************************\n");
    sleep(1);
  }

  system("clear");
  printf("***********************************\n");
  printf("*           Time's up!            *\n");
  printf("***********************************\n");
}

void date(void) { system("date"); }

void time_(void) {
  setenv("TZ", "Asia/Ho_Chi_Minh", 1);
  tzset();

  time_t t = time(NULL);
  if (t == (time_t)-1) {
    perror("Failed to get the current time");
    return;
  }

  struct tm tm_info;
  if (localtime_r(&t, &tm_info) == NULL) {
    perror("Failed to convert time to local time");
    return;
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
  printf("Current time: %s\n", buffer);
}

void addProcess(int pid, const char *name, int status) {
  (void)name;
  (void)status;
  printf("[Shell Info] Process added to background list: PID=%d\n", pid);
}

static void redirect_output_to_null(void) {
  freopen("/dev/null", "w", stderr);
  freopen("/dev/null", "w", stdout);
}

static void exec_calculator(int is_background) {
  if (is_background) {
    char *args[] = {"open", "-a", "Calculator", NULL};
    execvp("open", args);
  } else {
    char *args[] = {"open", "-W", "-a", "Calculator", NULL};
    execvp("open", args);
  }
  perror("execvp failed");
  _exit(1);
}

static void handle_child_process(int is_background) {
  redirect_output_to_null();
  exec_calculator(is_background);
}

static void handle_parent_background(pid_t pid) {
  addProcess(pid, "Calculator", 0);
  printf("Calculator opened in background with PID %d\n", pid);
}

void openCalculator(int is_background) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("Fork failed");
    return;
  }

  if (pid == 0) {
    setpgid(0, 0); // New process group
    if (!is_background) {
      tcsetpgrp(STDIN_FILENO, getpid()); // Take control
    }
    // Restore signals if needed (though open usage might not care)
    handle_child_process(is_background);
  } else {
    setpgid(pid, pid); // Ensure PGID set
    if (is_background) {
      handle_parent_background(pid);
    } else {
      tcsetpgrp(STDIN_FILENO, pid); // Give control
      handle_parent_foreground(pid);
      tcsetpgrp(STDIN_FILENO, getpgrp()); // Take control back
    }
  }
}