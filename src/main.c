#include "Builtins.h"
#include "execute.h"
#include "introduction.h"
#include "parser.h"
#include "shell.h"

pid_t fg_pid = 0;
char fg_command_name[256];
int history_count = 0;
char *history[HISTORY_MAX];

void add_to_history(char *line) {
  char *entry = strdup(line);
  entry[strcspn(entry, "\n")] = 0;
  if (history_count >= HISTORY_MAX) {
    free(history[history_count % HISTORY_MAX]);
  }
  history[history_count % HISTORY_MAX] = entry;
  history_count++;
}

void lsh_loop(void) {
  Token tokens[1024];
  char *line;
  // char **args;
  int status;

  do {
    printf("$ ");
    line = lsh_read_line();

    if (strcmp(line, "!!\n") == 0 || strcmp(line, "!!") == 0) {
      if (history_count == 0) {
        printf("glsh: no command in history yet.\n");
        free(line);
        continue;
      }

      char *last_cmd = history[(history_count - 1) % HISTORY_MAX];

      printf("%s\n", last_cmd);

      free(line);
      line = strdup(last_cmd);
    }

    if (line[0] != '\0' && line[0] != '\n')
      add_to_history(line);

    if (!line || strlen(line) == 0) {
      if (line)
        free(line);
      continue;
    }
    int n_tok = tokenize(line, tokens);
    if (n_tok > 0) {
      Pipeline *pl = parse(tokens, n_tok);

      if (pl->count > 0 && pl->cmds[0]->argc > 0) {
        if (strcmp(pl->cmds[0]->argv[0], "exit") == 0) {
          break;
        }
        status = execute_pipeline(pl);
      }
    }

    free(line);
  } while (status);
}

void sigchld_handler(int signum) {
  (void)signum;
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      delete_job(pid);
    } else if (WIFSTOPPED(status)) {
      Job *job = find_job(pid);
      if (job) {
        job->status = JOB_STOPPED;
        printf("\n[%d]+  Stopped                 %s\n", job->id, job->command);
      }
    } else if (WIFCONTINUED(status)) {
      Job *job = find_job(pid);
      if (job) {
        job->status = JOB_RUNNING;
      }
    }
  }
}

int main(int argc, char **args) {
  (void)argc;
  (void)args;

  int shell_terminal = STDIN_FILENO;
  int shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    while (tcgetpgrp(shell_terminal) != (getpgrp())) {
      kill(-getpgrp(), SIGTTIN);
    }

    // Ignore interactive and job-control signals
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    // Grab control of the terminal
    tcsetpgrp(shell_terminal, shell_pgid);
  }

  init_jobs();

  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags =
      SA_RESTART | SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }


  introduction();

  lsh_loop();


  return EXIT_SUCCESS;
}