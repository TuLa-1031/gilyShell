#include "shell.h"
#include "parser.h"

char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_SUCCESS);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If hit EOF, replace it with a null char and ret.
        if (c==EOF || c=='\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If exceed the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allcation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int tokenize(const char *line, Token *tokens) {
    /*
    intput: command string line
            tokens structrute
    output: number commad after split
    */
    int n = 0;
    char buf[MAX_TOKEN_LEN];
    int bi = 0;
    int len = strlen(line);
    int i = 0;
    int in_single = 0, in_double = 0;

    #define END_TOKEN() \
        if (bi > 0) { \
        buf[bi] = 0; \
        tokens[n].type = T_WORD; \
        tokens[n].value = strdup(buf); \
        n++; \
        bi = 0; \
    }

    while (i < len) {
        char c = line[i];

        if (!in_single && !in_double && isspace((unsigned char)c)) {
            END_TOKEN(); i++; continue;
        }

        if (in_single) {
            if (c=='\'') {in_single = 0; i++; continue;}
            buf[bi++] = c; i++; continue;
        }

        if (in_double) {
            if (c == '"') {in_double = 0; i++; continue;}
            if (c=='\\') { if (i+1 < len) {
                    char nxt = line[i+1];
                    if (nxt == '"' || nxt == '\\' || nxt == '$'){buf[bi++] = nxt; i+=2; continue;}
                }
            }
            buf[bi++] = c; i++; continue;
        }
        // operators
        if (c == '\'') {in_single = 1; i++; continue;}
        if (c == '"') {in_double = 1; i++; continue;}
        if (c == '\\') {
            if (i + 1 < len) { buf[bi++] = line[i+1]; i+=2; continue;}
            buf[bi++] = '\\'; i++; continue;
        }

        if (c == '|') {
            END_TOKEN();
            tokens[n].type = T_PIPE;
            tokens[n].value = NULL;
            n++; i++; continue;
        }

        if (c == '>') {
            END_TOKEN();
            if (i+1 < len && line[i+1] == '>') {
                tokens[n].type = T_REDIR_APPEND;
                tokens[n].value = NULL;
                n++; i+=2;
            }   else {
                tokens[n].type = T_REDIR_OUT;
                tokens[n].value = NULL;
                n++; i++;
            }
            continue;
        }
        if (c == '<') {
            END_TOKEN();
            tokens[n].type = T_REDIR_IN;
            tokens[n].value = NULL;
            n++; i++; continue;
        }
        buf[bi++] = c;
        i++;
    }
    END_TOKEN();
    return n;
}

Pipeline *parse(Token *toks, int ntok) {
    Pipeline *pl = calloc(1, sizeof(Pipeline));

    Command *cur = calloc(1, sizeof(Command));
    pl->cmds[pl->count++] = cur;

    for (int i = 0; i < ntok; i++) {
        Token t = toks[i];

        if (t.type == T_WORD) {
            cur->argv[cur->argc++] = strdup(t.value);
            cur->argv[cur->argc] = NULL;
        }   else if (t.type == T_PIPE) {
            //new cmd in pipline
            cur = calloc(1, sizeof(Command));
            pl->cmds[pl->count++] = cur;
        }   else if (t.type == T_REDIR_IN) {
            if (i+1 >= ntok || toks[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after <\n");
                return pl;
            }
            cur->in_file = strdup(toks[i+1].value);
            i++;
        }   else if (t.type == T_REDIR_OUT) {
            if (i+1 >= ntok || toks[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after >\n");
                return pl;
            }
            cur->out_file = strdup(toks[i+1].value);
            cur->append = 0;
            i++;
        }   else if (t.type == T_REDIR_APPEND) {
            if (i+1 >= ntok || toks[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after >>\n");
                return pl;
            }
            cur->out_file = strdup(toks[i+1].value);
            cur->append = 1;
            i++;
        }
    }   
    return pl;
}


int has_wildcard(const char *s) {
    return (strchr(s, '*') || strchr(s, '?') ||strchr(s, '['));
}


char **expand_to_glob_argv(char **args) {
    glob_t globbuf;
    char **new_argv = NULL;
    int new_argc = 0;

    for (int i = 0; args[i]!=NULL; i++) {
        if (has_wildcard((args[i]))) {
            int ret = glob(args[i], 0, NULL, &globbuf);
            if (ret==0) {
                for (size_t j = 0; j < globbuf.gl_pathc; j++) {
                    new_argv = realloc(new_argv, sizeof(char*)*(new_argc + 2));
                    new_argv[new_argc] = strdup(globbuf.gl_pathv[j]);
                    new_argc++;
                }
                globfree(&globbuf);
            }   else {
                new_argv = realloc(new_argv, sizeof(char*) * (new_argc + 2));
                new_argv[new_argc] = strdup(args[i]);
                new_argc++;
            }
        }   else {
            new_argv = realloc(new_argv, sizeof(char*) * (new_argc + 2));
            new_argv[new_argc] = strdup(args[i]);
            new_argc++;
        }
    }
    if (new_argv) {
        new_argv[new_argc] = NULL;
    }
    if (new_argv==NULL) return args;
    return new_argv;
}

char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position=0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "lsh: allcation arror\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token!=NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}