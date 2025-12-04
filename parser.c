#include "shell.h"
#include "parser.h"

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

    return new_argv;
}