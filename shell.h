#ifndef TINYSHELL_H
#define TINYSHELL_H


#include <sys/wait.h> //waitpid() and associated macros
#include <unistd.h> // chdir(), fork(), exec(), pid_t
#include <stdlib.h> // malloc(), realloc(), free(), exit(), excvp(), EXIT_SUCCES, EXIT_FAILURE
#include <stdio.h> // fprintf(), printf(), stderr, getchar(), perror()
#include <string.h> //strcmp(), strtok(),
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>


#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define HISTORY_MAX 20

extern char *history[HISTORY_MAX];
extern int history_count;

extern pid_t fg_pid;
char fg_command_name[256];

void printAnimatedText();

#endif