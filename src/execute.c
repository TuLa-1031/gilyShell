#include "Builtins.h"
#include "parser.h"
#include "shell.h"

static int is_builtin(const char *cmd, int *index) {
  for (int i = 0; i < glsh_num_builtins(); i++) {
    if (strcmp(cmd, glsh_builtin_names[i]) == 0) {
      *index = i;
      return 1;
    }
  }
  return 0;
}

int glsh_command_exists(const char *cmd) {
  int idx;
  if (is_builtin(cmd, &idx)) {
    return 1;
  }

  if (!glsh_exec_enabled) {
    return 0;
  }

  if (strchr(cmd, '/') != NULL) {
    return access(cmd, X_OK) == 0;
  }

  char *path_env = getenv("PATH");
  if (!path_env) {
    return 0;
  }

  char *path_copy = strdup(path_env);
  char *dir = strtok(path_copy, ":");
  char full_path[1024];

  while (dir != NULL) {
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
    if (access(full_path, X_OK) == 0) {
      free(path_copy);
      return 1;
    }
    dir = strtok(NULL, ":");
  }

  free(path_copy);
  return 0;
}

static int run_builtin(int index, char **args) {
  return (*glsh_builtin_funcs[index])(args);
}

static void setup_input_redirect(Command *cmd, int prev_pipe_read) {
  if (cmd->in_file) {
    int fd = open(cmd->in_file, O_RDONLY);
    if (fd == -1) {
      perror(cmd->in_file);
      _exit(EXIT_FAILURE);
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
      _exit(EXIT_FAILURE);
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

  if (is_builtin(argv[0], &builtin_idx)) {
    run_builtin(builtin_idx, argv);
    _exit(EXIT_SUCCESS);
  }

  execvp(argv[0], argv);
  fprintf(stderr, "glsh: %s: command not found\n", argv[0]);
  _exit(EXIT_FAILURE);
}

int glsh_execute_pipeline(Pipeline *pl) {
  int prev_pipe_read = -1;
  int pipefd[2];
  pid_t pgid = 0;
  char job_cmd[256] = "";

  if (pl->count == 1 && pl->cmds[0]->argv[0] != NULL) {
    int builtin_idx;
    if (is_builtin(pl->cmds[0]->argv[0], &builtin_idx)) {
      if (strcmp(pl->cmds[0]->argv[0], "countd") == 0) {
        return glsh_builtin_countd(pl->cmds[0]->argv, pl->background);
      }
      if (strcmp(pl->cmds[0]->argv[0], "calculator") == 0) {
        return glsh_builtin_calculator(pl->cmds[0]->argv, pl->background);
      }
      if (strcmp(pl->cmds[0]->argv[0], "repeat") == 0) {
        return glsh_builtin_repeat(pl->cmds[0]->argv, pl->background);
      }
      if (!pl->background) {
        return run_builtin(builtin_idx, pl->cmds[0]->argv);
      }
    }
  }

  if (pl->background && pl->count > 0 && pl->cmds[0]->argv[0]) {
    for (int i = 0; i < pl->cmds[0]->argc && pl->cmds[0]->argv[i]; i++) {
      if (i > 0) {
        strncat(job_cmd, " ", sizeof(job_cmd) - strlen(job_cmd) - 1);
      }
      strncat(job_cmd, pl->cmds[0]->argv[i], sizeof(job_cmd) - strlen(job_cmd) - 1);
    }
  }

  for (int i = 0; i < pl->count; i++) {
    Command *cmd = pl->cmds[i];
    char **expanded_argv = glsh_expand_glob(cmd->argv);
    int is_last = (i == pl->count - 1);
    int needs_free_expanded = (expanded_argv != cmd->argv);

    if (expanded_argv[0] == NULL) {
      if (needs_free_expanded) {
        for (int j = 0; expanded_argv[j]; j++) {
          free(expanded_argv[j]);
        }
        free(expanded_argv);
      }
      continue;
    }

    if (!is_last && pipe(pipefd) == -1) {
      perror("glsh: pipe");
      if (needs_free_expanded) {
        for (int j = 0; expanded_argv[j]; j++) {
          free(expanded_argv[j]);
        }
        free(expanded_argv);
      }
      return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
      if (pgid == 0) {
        setpgid(0, 0);
      } else {
        setpgid(0, pgid);
      }

      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
      signal(SIGCHLD, SIG_DFL);

      if (pl->background) {
        freopen("/dev/null", "r", stdin);
      } else if (is_last) {
        tcsetpgrp(STDIN_FILENO, pgid == 0 ? getpid() : pgid);
      }

      setup_input_redirect(cmd, prev_pipe_read);
      setup_output_redirect(cmd, pipefd, is_last);

      if (!is_last) {
        close(pipefd[0]);
      }

      execute_child(expanded_argv);
    } else if (pid < 0) {
      perror("glsh: fork");
      if (needs_free_expanded) {
        for (int j = 0; expanded_argv[j]; j++) {
          free(expanded_argv[j]);
        }
        free(expanded_argv);
      }
      return 1;
    } else {
      if (pgid == 0) {
        pgid = pid;
      }
      setpgid(pid, pgid);

      if (prev_pipe_read != -1) {
        close(prev_pipe_read);
      }

      if (!is_last) {
        close(pipefd[1]);
        prev_pipe_read = pipefd[0];
      }

      if (is_last && pl->background) {
        glsh_job_add(pgid, JOB_RUNNING, job_cmd);
      }

      if (needs_free_expanded) {
        for (int j = 0; expanded_argv[j]; j++) {
          free(expanded_argv[j]);
        }
        free(expanded_argv);
      }
    }
  }

  if (!pl->background) {
    if (pgid > 0) {
      tcsetpgrp(STDIN_FILENO, pgid);
    }
    for (int i = 0; i < pl->count; i++) {
      int status;
      waitpid(-1, &status, WUNTRACED);
    }
    tcsetpgrp(STDIN_FILENO, getpgrp());
  }

  return 1;
}