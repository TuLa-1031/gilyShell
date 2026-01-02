#include "Builtins.h"
#include "parser.h"
#include "shell.h"

static int is_builtin(const char *cmd, int *index) {
  for (int i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(cmd, builtin_str[i]) == 0) {
      *index = i;
      return 1;
    }
  }
  return 0;
}

static int run_builtin(int index, char **args) {
  return (*builtin_func[index])(args);
}

static void setup_input_redirect(Command *cmd, int prev_pipe_read) {
  if (cmd->in_file) {
    int fd = open(cmd->in_file, O_RDONLY);
    if (fd == -1) {
      perror(cmd->in_file);
      exit(EXIT_FAILURE);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
  } else if (prev_pipe_read != -1) {
    dup2(prev_pipe_read, STDIN_FILENO);
    close(prev_pipe_read);
  }
}

static void setup_output_redirect(Command *cmd, int pipefd[2], int is_last) {
  if (cmd->out_file) {
    int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
    int fd = open(cmd->out_file, flags, 0644);
    if (fd == -1) {
      perror(cmd->out_file);
      exit(EXIT_FAILURE);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
  } else if (!is_last) {
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
  }
}

static void execute_child(char **argv) {
  int builtin_idx;

  // DEBUG: Print what we are trying to execute

  if (is_builtin(argv[0], &builtin_idx)) {

    run_builtin(builtin_idx, argv);
    exit(EXIT_SUCCESS);
  }

  if (execvp(argv[0], argv) == -1) {
    perror("lsh");
  }
  exit(EXIT_FAILURE);
}

int lsh_execute(char **args) {
  if (args[0] == NULL) {
    return 1;
  }

  int builtin_idx;
  if (is_builtin(args[0], &builtin_idx)) {
    return run_builtin(builtin_idx, args);
  }

  pid_t pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("lsh");
  } else {
    int status;
    waitpid(pid, &status, WUNTRACED);
    while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
      waitpid(pid, &status, WUNTRACED);
    }
  }

  return 1;
}

int execute_pipeline(Pipeline *pl) {
  int prev_pipe_read = -1;
  int pipefd[2];

  if (pl->count == 1 && pl->cmds[0]->argv[0] != NULL && !pl->background) {
    int builtin_idx;
    if (is_builtin(pl->cmds[0]->argv[0], &builtin_idx)) {
      return run_builtin(builtin_idx, pl->cmds[0]->argv);
    }
  }

  if (pl->background) {
    if (pl->count > 0 && pl->cmds[pl->count - 1]->argv[0] != NULL) {
      // Ideally we'd want the PID of the last command in pipeline if it's a
      // pipeline But simplified for now, we just don't wait for any. We need to
      // track the last PID to add to jobs.
    }
  }

  for (int i = 0; i < pl->count; i++) {
    Command *cmd = pl->cmds[i];
    char **expanded_argv = expand_to_glob_argv(cmd->argv);
    int is_last = (i == pl->count - 1);

    if (expanded_argv[0] == NULL) {
      continue;
    }

    if (!is_last && pipe(pipefd) == -1) {
      perror("pipe");
      return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
      // Child process
      setpgid(0, 0); // Put child in its own process group

      if (!pl->background && is_last) {
        // If foreground, give terminal control to this process group
        tcsetpgrp(STDIN_FILENO, getpid());
      }

      // Restore default signal handlers for child
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      signal(SIGCHLD, SIG_DFL);

      setup_input_redirect(cmd, prev_pipe_read);
      setup_output_redirect(cmd, pipefd, is_last);

      if (!is_last) {
        close(pipefd[0]);
      }

      execute_child(expanded_argv);
    } else if (pid < 0) {
      perror("fork");
      return 1;
    } else {
      // Parent process
      setpgid(pid,
              pid); // Ensure child is in its own group (avoid race condition)
      if (prev_pipe_read != -1) {
        close(prev_pipe_read);
      }

      if (!is_last) {
        close(pipefd[1]);
        prev_pipe_read = pipefd[0];
      }

      // If this is the last command in a background pipeline, add it to jobs
      if (is_last && pl->background) {
        add_job(pid, JOB_RUNNING, expanded_argv[0]); // Simple command name
      }
    }
  }

  if (!pl->background) {
    // Foreground: wait for all
    for (int i = 0; i < pl->count; i++) {
      int status;
      waitpid(-1, &status, WUNTRACED);
    }
    // Restore terminal control to shell
    tcsetpgrp(STDIN_FILENO, getpgrp());
  }

  return 1;
}