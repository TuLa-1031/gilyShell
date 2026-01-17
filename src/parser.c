#include "shell.h"
#include <errno.h>

static void allocation_error(void) {
  fprintf(stderr, "glsh: allocation error\n");
  exit(EXIT_FAILURE);
}

char *glsh_read_line(void) {
  int bufsize = GLSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  char ch;

  if (!buffer) {
    allocation_error();
  }

  while (1) {
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    
    if (n == 0) {
      buffer[position] = '\0';
      return buffer;
    } else if (n == -1) {
      if (errno == EINTR || errno == EIO) {
        continue;
      }
      free(buffer);
      buffer = malloc(1);
      if (buffer) {
        buffer[0] = '\0';
      }
      return buffer;
    }

    if (ch == '\n') {
      buffer[position] = '\0';
      return buffer;
    }

    buffer[position++] = ch;

    if (position >= bufsize) {
      bufsize += GLSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        allocation_error();
      }
    }
  }
}

static void end_token(char *buf, int *buf_index, Token *tokens, int *n) {
  if (*buf_index > 0) {
    buf[*buf_index] = '\0';
    tokens[*n].type = TOKEN_WORD;
    tokens[*n].value = strdup(buf);
    (*n)++;
    *buf_index = 0;
  }
}

static void add_operator_token(Token *tokens, int *n, TokenType type) {
  tokens[*n].type = type;
  tokens[*n].value = NULL;
  (*n)++;
}

int glsh_tokenize(const char *line, Token *tokens) {
  int n = 0;
  char buf[GLSH_MAX_TOKEN];
  int buf_index = 0;
  int len = strlen(line);
  int in_single = 0, in_double = 0;

  for (int i = 0; i < len; i++) {
    char c = line[i];

    if (!in_single && !in_double && isspace((unsigned char)c)) {
      end_token(buf, &buf_index, tokens, &n);
      continue;
    }

    if (in_single) {
      if (c == '\'') {
        in_single = 0;
      } else {
        buf[buf_index++] = c;
      }
      continue;
    }

    if (in_double) {
      if (c == '"') {
        in_double = 0;
      } else if (c == '\\' && i + 1 < len) {
        char nxt = line[i + 1];
        if (nxt == '"' || nxt == '\\' || nxt == '$') {
          buf[buf_index++] = nxt;
          i++;
        } else {
          buf[buf_index++] = c;
        }
      } else {
        buf[buf_index++] = c;
      }
      continue;
    }

    switch (c) {
    case '\'':
      in_single = 1;
      break;
    case '"':
      in_double = 1;
      break;
    case '\\':
      if (i + 1 < len) {
        buf[buf_index++] = line[++i];
      } else {
        buf[buf_index++] = '\\';
      }
      break;
    case '|':
      end_token(buf, &buf_index, tokens, &n);
      add_operator_token(tokens, &n, TOKEN_PIPE);
      break;
    case '>':
      end_token(buf, &buf_index, tokens, &n);
      if (i + 1 < len && line[i + 1] == '>') {
        add_operator_token(tokens, &n, TOKEN_REDIR_APPEND);
        i++;
      } else {
        add_operator_token(tokens, &n, TOKEN_REDIR_OUT);
      }
      break;
    case '<':
      end_token(buf, &buf_index, tokens, &n);
      add_operator_token(tokens, &n, TOKEN_REDIR_IN);
      break;
    case '&':
      end_token(buf, &buf_index, tokens, &n);
      add_operator_token(tokens, &n, TOKEN_BACKGROUND);
      break;
    default:
      buf[buf_index++] = c;
    }
  }
  end_token(buf, &buf_index, tokens, &n);
  return n;
}

static char *parse_redir_file(Token *toks, int ntok, int *i, const char *op) {
  if (*i + 1 >= ntok || toks[*i + 1].type != TOKEN_WORD) {
    fprintf(stderr, "glsh: syntax error: expected file after %s\n", op);
    return NULL;
  }
  (*i)++;
  return strdup(toks[*i].value);
}

Pipeline *glsh_parse(Token *toks, int ntok) {
  Pipeline *pl = calloc(1, sizeof(Pipeline));
  Command *cur = calloc(1, sizeof(Command));
  pl->cmds[pl->count++] = cur;

  for (int i = 0; i < ntok; i++) {
    Token t = toks[i];

    switch (t.type) {
    case TOKEN_WORD:
      cur->argv[cur->argc++] = strdup(t.value);
      cur->argv[cur->argc] = NULL;
      break;
    case TOKEN_PIPE:
      cur = calloc(1, sizeof(Command));
      pl->cmds[pl->count++] = cur;
      break;
    case TOKEN_REDIR_IN:
      cur->in_file = parse_redir_file(toks, ntok, &i, "<");
      if (!cur->in_file) {
        return pl;
      }
      break;
    case TOKEN_REDIR_OUT:
      cur->out_file = parse_redir_file(toks, ntok, &i, ">");
      if (!cur->out_file) {
        return pl;
      }
      cur->append = 0;
      break;
    case TOKEN_REDIR_APPEND:
      cur->out_file = parse_redir_file(toks, ntok, &i, ">>");
      if (!cur->out_file) {
        return pl;
      }
      cur->append = 1;
      break;
    case TOKEN_BACKGROUND:
      pl->background = 1;
      break;
    }
  }
  return pl;
}

static int has_wildcard(const char *s) {
  return strchr(s, '*') || strchr(s, '?') || strchr(s, '[');
}

static void append_arg(char ***argv, int *argc, const char *arg) {
  *argv = realloc(*argv, sizeof(char *) * (*argc + 2));
  (*argv)[(*argc)++] = strdup(arg);
}

char **glsh_expand_glob(char **args) {
  char **new_argv = NULL;
  int new_argc = 0;
  glob_t globbuf;

  for (int i = 0; args[i]; i++) {
    if (has_wildcard(args[i]) && glob(args[i], 0, NULL, &globbuf) == 0) {
      for (size_t j = 0; j < globbuf.gl_pathc; j++) {
        append_arg(&new_argv, &new_argc, globbuf.gl_pathv[j]);
      }
      globfree(&globbuf);
    } else {
      append_arg(&new_argv, &new_argc, args[i]);
    }
  }

  if (new_argv) {
    new_argv[new_argc] = NULL;
  }

  return new_argv ? new_argv : args;
}

void glsh_tokens_free(Token *tokens, int count) {
  for (int i = 0; i < count; i++) {
    if (tokens[i].value) {
      free(tokens[i].value);
      tokens[i].value = NULL;
    }
  }
}

void glsh_pipeline_free(Pipeline *pl) {
  if (!pl) {
    return;
  }
  for (int i = 0; i < pl->count; i++) {
    Command *cmd = pl->cmds[i];
    if (cmd) {
      for (int j = 0; j < cmd->argc; j++) {
        free(cmd->argv[j]);
      }
      free(cmd->in_file);
      free(cmd->out_file);
      free(cmd);
    }
  }
  free(pl);
}
