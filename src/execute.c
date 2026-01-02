#include "Builtins.h"
#include "parser.h"
#include "shell.h"

int lsh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // AN empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  // printf("lsh: No such file or directory\n");
  // free(history[history_count]);
  // history_count--;
  // if (history_count < 0) history_count = HISTORY_MAX;
  // return 1;
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFEXITED(status));
  }
  return 1;
}

int execute_pipeline(Pipeline *pl) {
  int i;
  int prev_pipe_read = -1;
  int pipefd[2];
  pid_t pid;

  if (pl->count == 1 && pl->cmds[0]->argv[0] != NULL) {
    char **raw_args = pl->cmds[0]->argv;
    for (i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(raw_args[0], builtin_str[i]) == 0) {
        // Chạy hàm built-in (vd: lsh_cd) và trả về status
        return (*builtin_func[i])(raw_args);
      }
    }
  }

  for (i = 0; i < pl->count; i++) {
    Command *cmd = pl->cmds[i];

    char **expanded_argv = expand_to_glob_argv(cmd->argv);
    if (expanded_argv[0] == NULL) {
      continue;
    }

    if (i < pl->count - 1) {
      if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
      }
    }

    pid = fork();
    if (pid == 0) {
      // child
      // handle input
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

      // handle output
      if (cmd->out_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;
        int fd = open(cmd->out_file, flags, 0644);
        if (fd == -1) {
          perror(cmd->out_file);
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      } else if (i < pl->count - 1) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
      }

      if (i < pl->count - 1)
        close(pipefd[0]);

      // exec
      for (int j = 0; j < lsh_num_builtins(); j++) {
        if (strcmp(expanded_argv[0], builtin_str[j]) == 0) {
          (*builtin_func[j])(expanded_argv);
          exit(EXIT_SUCCESS); // Child xong việc thì phải chết
        }
      }

      if (execvp(expanded_argv[0], expanded_argv) == -1) {
        perror("lsh");
      }
      exit(EXIT_FAILURE);

    } else if (pid < 0) {
      perror("fork");
      return 1;
    } else {
      // parent
      if (prev_pipe_read != -1)
        close(prev_pipe_read);

      if (i < pl->count - 1) {
        close(pipefd[1]);
        prev_pipe_read = pipefd[0];
      }
    }
  }

  for (i = 0; i < pl->count; i++) {
    wait(NULL);
  }

  return 1;
}