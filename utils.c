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
    printf("*           HHH!           *\n");
    printf("***********************************\n");
}

void date(){
    system("date");
}

void time_() {
    setenv("TZ", "Asia/Ho_Chi_Minh", 1); // Set Vietnam timezone
    tzset(); // Apply the new timezone

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