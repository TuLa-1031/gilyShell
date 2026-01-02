#include "parser.h"
#include "shell.h"

static void allocation_error(const char *prefix) {
  fprintf(stderr, "%s: allocation error\n", prefix);
  exit(EXIT_FAILURE);
}

char *lsh_read_line(void) {
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer)
    allocation_error("lsh");

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }

    buffer[position++] = c;

    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
        allocation_error("glsh");
    }
  }
}

static void end_token(char *buf, int *buf_index, Token *tokens, int *n) {
  if (*buf_index > 0) {
    buf[*buf_index] = '\0';
    tokens[*n].type = T_WORD;
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

int tokenize(const char *line, Token *tokens) {
  int n = 0;
  char buf[MAX_TOKEN_LEN];
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
      if (c == '\'')
        in_single = 0;
      else
        buf[buf_index++] = c;
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
      if (i + 1 < len)
        buf[buf_index++] = line[++i];
      else
        buf[buf_index++] = '\\';
      break;
    case '|':
      end_token(buf, &buf_index, tokens, &n);
      add_operator_token(tokens, &n, T_PIPE);
      break;
    case '>':
      end_token(buf, &buf_index, tokens, &n);
      if (i + 1 < len && line[i + 1] == '>') {
        add_operator_token(tokens, &n, T_REDIR_APPEND);
        i++;
      } else {
        add_operator_token(tokens, &n, T_REDIR_OUT);
      }
      break;
    case '<':
      end_token(buf, &buf_index, tokens, &n);
      add_operator_token(tokens, &n, T_REDIR_IN);
      break;
    default:
      buf[buf_index++] = c;
    }
  }
  end_token(buf, &buf_index, tokens, &n);
  return n;
}

static char *parse_redir_file(Token *toks, int ntok, int *i, const char *op) {
  if (*i + 1 >= ntok || toks[*i + 1].type != T_WORD) {
    fprintf(stderr, "syntax error: expected file after %s\n", op);
    return NULL;
  }
  (*i)++;
  return strdup(toks[*i].value);
}

Pipeline *parse(Token *toks, int ntok) {
  Pipeline *pl = calloc(1, sizeof(Pipeline));
  Command *cur = calloc(1, sizeof(Command));
  pl->cmds[pl->count++] = cur;

  for (int i = 0; i < ntok; i++) {
    Token t = toks[i];

    switch (t.type) {
    case T_WORD:
      cur->argv[cur->argc++] = strdup(t.value);
      cur->argv[cur->argc] = NULL;
      break;
    case T_PIPE:
      cur = calloc(1, sizeof(Command));
      pl->cmds[pl->count++] = cur;
      break;
    case T_REDIR_IN:
      cur->in_file = parse_redir_file(toks, ntok, &i, "<");
      if (!cur->in_file) return pl;
      break;
    case T_REDIR_OUT:
      cur->out_file = parse_redir_file(toks, ntok, &i, ">");
      if (!cur->out_file) return pl;
      cur->append = 0;
      break;
    case T_REDIR_APPEND:
      cur->out_file = parse_redir_file(toks, ntok, &i, ">>");
      if (!cur->out_file) return pl;
      cur->append = 1;
      break;
    }
  }
  return pl;
}

int has_wildcard(const char *s) {
  return strchr(s, '*') || strchr(s, '?') || strchr(s, '[');
}

static void append_arg(char ***argv, int *argc, const char *arg) {
  *argv = realloc(*argv, sizeof(char *) * (*argc + 2));
  (*argv)[(*argc)++] = strdup(arg);
}

char **expand_to_glob_argv(char **args) {
  char **new_argv = NULL;
  int new_argc = 0;
  glob_t globbuf;

  for (int i = 0; args[i]; i++) {
    if (has_wildcard(args[i]) && glob(args[i], 0, NULL, &globbuf) == 0) {
      for (size_t j = 0; j < globbuf.gl_pathc; j++)
        append_arg(&new_argv, &new_argc, globbuf.gl_pathv[j]);
      globfree(&globbuf);
    } else {
      append_arg(&new_argv, &new_argc, args[i]);
    }
  }

  if (new_argv)
    new_argv[new_argc] = NULL;

  return new_argv ? new_argv : args;
}

char **lsh_split_line(char *line) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));

  if (!tokens)
    allocation_error("glsh");

  char *token = strtok(line, LSH_TOK_DELIM);
  while (token) {
    tokens[position++] = token;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      char **tmp = realloc(tokens, bufsize * sizeof(char *));
      if (!tmp) {
        free(tokens);
        allocation_error("lsh");
      }
      tokens = tmp;
    }
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
