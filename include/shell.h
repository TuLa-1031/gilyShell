#ifndef TINYSHELL_H
#define TINYSHELL_H

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h> // fprintf(), printf(), stderr, getchar(), perror()
#include <stdlib.h> // malloc(), realloc(), free(), exit(), excvp(), EXIT_SUCCES, EXIT_FAILURE
#include <string.h> //strcmp(), strtok(),
#include <sys/types.h>
#include <sys/wait.h> //waitpid() and associated macros
#include <time.h>
#include <unistd.h> // chdir(), fork(), exec(), pid_t

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define HISTORY_MAX 20
#define MAX_TOKEN_LEN 1024
#define MAX_ARGS 64
#define MAX_CMDS 16

typedef enum {
  T_WORD,
  T_PIPE,
  T_REDIR_IN,
  T_REDIR_OUT,
  T_REDIR_APPEND
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

typedef struct {
  char *in_file;
  char *out_file;
  int append;
  char *argv[MAX_ARGS];
  int argc;
} Command;

typedef struct {
  Command *cmds[MAX_CMDS];
  int count;
} Pipeline;

extern char *history[HISTORY_MAX];
extern int history_count;

extern pid_t fg_pid;
extern char fg_command_name[256];

extern char **environ;

void printAnimatedText();

#endif