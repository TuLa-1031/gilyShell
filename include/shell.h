#ifndef GLSH_SHELL_H
#define GLSH_SHELL_H

#include <ctype.h>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define GLSH_RL_BUFSIZE   1024
#define GLSH_TOK_BUFSIZE  64
#define GLSH_TOK_DELIM    " \t\r\n\a"
#define GLSH_HISTORY_MAX  20
#define GLSH_MAX_TOKEN    1024
#define GLSH_MAX_ARGS     64
#define GLSH_MAX_CMDS     16
#define GLSH_MAX_JOBS     64

typedef enum {
  TOKEN_WORD,
  TOKEN_PIPE,
  TOKEN_REDIR_IN,
  TOKEN_REDIR_OUT,
  TOKEN_REDIR_APPEND,
  TOKEN_BACKGROUND
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

typedef struct {
  char *in_file;
  char *out_file;
  int append;
  char *argv[GLSH_MAX_ARGS];
  int argc;
} Command;

typedef struct {
  Command *cmds[GLSH_MAX_CMDS];
  int count;
  int background;
} Pipeline;

typedef enum {
  JOB_RUNNING,
  JOB_STOPPED,
  JOB_DONE
} JobStatus;

typedef struct {
  pid_t pid;
  int id;
  JobStatus status;
  char *command;
} Job;

extern char *glsh_history[GLSH_HISTORY_MAX];
extern int glsh_history_count;
extern int glsh_exec_enabled;
extern char **environ;

void glsh_print_intro(void);

void glsh_jobs_init(void);
void glsh_job_add(pid_t pid, JobStatus status, const char *cmd);
void glsh_job_delete(pid_t pid);
Job *glsh_job_find(pid_t pid);
Job *glsh_job_find_by_id(int id);
void glsh_jobs_print(void);
int glsh_job_next_id(void);

void glsh_pipeline_free(Pipeline *pl);
void glsh_tokens_free(Token *tokens, int count);
int glsh_command_exists(const char *cmd);

#endif