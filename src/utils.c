#include "shell.h"

void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    system("clear"); // Linux terminal clear
    printf("***********************************\n");
    printf("*        Countdown Timer         *\n");
    printf("***********************************\n");
    printf("*          %2d seconds           *\n", i);
    printf("***********************************\n");
    sleep(1); // 1 second pause
  }

  system("clear");
  printf("***********************************\n");
  printf("*           Time out!           *\n");
  printf("***********************************\n");
}

void date() { system("date"); }

void time_() {
  setenv("TZ", "Asia/Ho_Chi_Minh", 1); // Set Vietnam timezone
  tzset();                             // Apply the new timezone

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

void addProcess(int pid, char *name, int status) {
  printf("[Shell Info] Process added to background list: PID=%d\n", pid);
}

void openCalculator(int is_background) {
  pid_t pid = fork();

  if (pid == 0) {
    // --- TIẾN TRÌNH CON ---

    // Redirect stderr để terminal không bị rác nếu lệnh open bắn log
    freopen("/dev/null", "w", stderr);
    freopen("/dev/null", "w", stdout);

    if (!is_background) {
      // FOREGROUND: Dùng tham số -W (Wait) để lệnh open đợi app tắt rồi mới kết
      // thúc Lệnh tương đương: open -W -a Calculator
      char *args[] = {"open", "-W", "-a", "Calculator", NULL};
      execvp("open", args);
    } else {
      // BACKGROUND: Chỉ mở lên rồi trả về ngay lập tức
      // Lệnh tương đương: open -a Calculator
      char *args[] = {"open", "-a", "Calculator", NULL};
      execvp("open", args);
    }

    // Nếu execvp chạy tới đây nghĩa là lỗi
    exit(1);

  } else if (pid > 0) {
    // --- TIẾN TRÌNH CHA ---

    if (!is_background) {
      // Chế độ Foreground
      fg_pid = pid;
      strcpy(fg_command_name, "Calculator");

      int status = 0;
      // Shell đợi lệnh "open -W" kết thúc (lệnh này kết thúc khi bạn tắt
      // Calculator)
      waitpid(pid, &status, WUNTRACED);

      fg_pid = -1;
    } else {
      // Chế độ Background
      addProcess(pid, "Calculator", 0);
      printf("Calculator opened in background with PID %d\n", pid);
    }

  } else {
    perror("Fork failed");
  }
}