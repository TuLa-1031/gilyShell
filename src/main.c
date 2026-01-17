#include "Builtins.h"
#include "execute.h"
#include "introduction.h"
#include "parser.h"
#include "shell.h"

int glsh_history_count = 0;
char *glsh_history[GLSH_HISTORY_MAX];
int glsh_exec_enabled = 1;

static volatile sig_atomic_t sigchld_received = 0;

static void sigchld_handler(int signum) {
  (void)signum;
  sigchld_received = 1;
}

static void reap_children(void) {
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      glsh_job_delete(pid);
    } else if (WIFSTOPPED(status)) {
      Job *job = glsh_job_find(pid);
      if (job) {
        job->status = JOB_STOPPED;
        printf("\n[%d]+  Stopped                 %s\n", job->id, job->command);
      }
    } else if (WIFCONTINUED(status)) {
      Job *job = glsh_job_find(pid);
      if (job) {
        job->status = JOB_RUNNING;
      }
    }
  }
  sigchld_received = 0;
}

static void add_to_history(const char *line) {
  char *entry = strdup(line);
  entry[strcspn(entry, "\n")] = 0;
  
  if (glsh_history_count >= GLSH_HISTORY_MAX) {
    free(glsh_history[glsh_history_count % GLSH_HISTORY_MAX]);
  }
  glsh_history[glsh_history_count % GLSH_HISTORY_MAX] = entry;
  glsh_history_count++;
}

static void glsh_loop(void) {
  Token tokens[1024];
  char *line;
  int status = 1;
  int n_tok;

  do {
    if (sigchld_received) {
      reap_children();
    }
    printf("$ ");
    fflush(stdout);
    line = glsh_read_line();

    if (!line) {
      break;
    }

    if (strcmp(line, "!!\n") == 0 || strcmp(line, "!!") == 0) {
      if (glsh_history_count == 0) {
        printf("glsh: no command in history yet.\n");
        free(line);
        continue;
      }

      char *last_cmd = glsh_history[(glsh_history_count - 1) % GLSH_HISTORY_MAX];

      printf("%s\n", last_cmd);

      free(line);
      line = strdup(last_cmd);
    }

    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    n_tok = glsh_tokenize(line, tokens);
    if (n_tok > 0) {
      Pipeline *pl = glsh_parse(tokens, n_tok);

      if (pl->count > 0 && pl->cmds[0]->argc > 0) {
        if (strcmp(pl->cmds[0]->argv[0], "exit") == 0) {
          glsh_pipeline_free(pl);
          glsh_tokens_free(tokens, n_tok);
          free(line);
          break;
        }
        if (glsh_command_exists(pl->cmds[0]->argv[0])) {
          if (line[0] != '\0' && line[0] != '\n') {
            add_to_history(line);
          }
          status = glsh_execute_pipeline(pl);
        } else {
          fprintf(stderr, "glsh: %s: command not found\n", pl->cmds[0]->argv[0]);
        }
      }
      glsh_pipeline_free(pl);
      glsh_tokens_free(tokens, n_tok);
    }

    free(line);
  } while (status);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  int shell_terminal = STDIN_FILENO;
  int shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    while (tcgetpgrp(shell_terminal) != getpgrp()) {
      kill(-getpgrp(), SIGTTIN);
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
      perror("glsh: couldn't put shell in its own process group");
      exit(EXIT_FAILURE);
    }

    tcsetpgrp(shell_terminal, shell_pgid);
  }

  glsh_jobs_init();

  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("glsh: sigaction");
    exit(EXIT_FAILURE);
  }

  glsh_introduction();
  glsh_loop();

  return EXIT_SUCCESS;
}