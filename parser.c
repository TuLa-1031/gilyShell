#include "./include/shell.h"
#include "./include/parser.h"

char *lsh_read_line(void) {
    int  buff_size = LSH_RL_BUFSIZE;
    int  position = 0;
    char *buffer = malloc(sizeof(char) * buff_size);
    int  current_char;

    if (!buffer) {
        fprintf(__stderrp, "glsh: allocation error\n");
        exit(EXIT_SUCCESS);
    }

    while (1) {
        // Read a character
        current_char = getchar();

        // If hit EOF, replace it with a null char and ret.
        if (current_char==EOF || current_char=='\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = current_char;
        }
        position++;

        // If exceed the buffer, reallocate.
        if (position >= buff_size) {
            buff_size += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, buff_size);
            if (!buffer) {
                fprintf(stderr, "glsh: allcation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int tokenize(const char *line, Token *tokens) {
    /*
    intput: command string line
    output: number command after split
    */
    int  n = 0;
    char buffer[MAX_TOKEN_LEN];
    int  buffer_index = 0;
    int  len = strlen(line);
    int  i = 0;
    int  in_single = 0, in_double = 0;

    #define END_TOKEN() \
        if (buffer_index > 0) { \
        buffer[buffer_index] = 0; \
        tokens[n].type = T_WORD; \
        tokens[n].value = strdup(buffer); \
        n++; \
        buffer_index = 0; \
    }

    while (i < len) {
        char character = line[i];

        if (!in_single && !in_double && isspace((unsigned char)character)) {
            END_TOKEN(); i++; continue;
        }

        if (in_single) {
            if (character=='\'') {in_single = 0; i++; continue;}
            buffer[buffer_index++] = character; i++; continue;
        }

        if (in_double) {
            if (character == '"') {in_double = 0; i++; continue;}
            if (character=='\\') { if (i+1 < len) {
                    char nxt = line[i+1];
                    if (nxt == '"' || nxt == '\\' || nxt == '$'){buffer[buffer_index++] = nxt; i+=2; continue;}
                }
            }
            buffer[buffer_index++] = character; i++; continue;
        }
        // operators
        if (character == '\'') {in_single = 1; i++; continue;}
        if (character == '"') {in_double = 1; i++; continue;}
        if (character == '\\') {
            if (i + 1 < len) { buffer[buffer_index++] = line[i+1]; i+=2; continue;}
            buffer[buffer_index++] = '\\'; i++; continue;
        }

        if (character == '|') {
            END_TOKEN();
            tokens[n].type = T_PIPE;
            tokens[n].value = NULL;
            n++; i++; continue;
        }

        if (character == '>') {
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
        if (character == '<') {
            END_TOKEN();
            tokens[n].type = T_REDIR_IN;
            tokens[n].value = NULL;
            n++; i++; continue;
        }
        buffer[buffer_index++] = character;
        i++;
    }
    END_TOKEN();
    return n;
}

Pipeline *parse(Token *tokens, int number_of_tokens) {
    Pipeline *pipeline = calloc(1, sizeof(Pipeline));

    Command *current_command = calloc(1, sizeof(Command));
    pipeline->commands[pipeline->count++] = current_command;

    for (int i = 0; i < number_of_tokens; i++) {
        Token t = tokens[i];

        if (t.type == T_WORD) {
            current_command->argv[current_command->argc++] = strdup(t.value);
            current_command->argv[current_command->argc] = NULL;
        }   else if (t.type == T_PIPE) {
            //new cmd in pipline
            current_command = calloc(1, sizeof(Command));
            pipeline->commands[pipeline->count++] = current_command;
        }   else if (t.type == T_REDIR_IN) {
            if (i+1 >= number_of_tokens || tokens[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after <\n");
                return pipeline;
            }
            current_command->in_file = strdup(tokens[i+1].value);
            i++;
        }   else if (t.type == T_REDIR_OUT) {
            if (i+1 >= number_of_tokens || tokens[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after >\n");
                return pipeline;
            }
            current_command->out_file = strdup(tokens[i+1].value);
            current_command->append = 0;
            i++;
        }   else if (t.type == T_REDIR_APPEND) {
            if (i+1 >= number_of_tokens || tokens[i+1].type != T_WORD) {
                fprintf(stderr, "syntax error: expected file after >>\n");
                return pipeline;
            }
            current_command->out_file = strdup(tokens[i+1].value);
            current_command->append = 1;
            i++;
        }
    }   
    return pipeline;
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
    int buff_size = LSH_TOK_BUFSIZE, position=0;
    char **tokens = malloc(buff_size * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "glsh: allcation arror\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token!=NULL) {
        tokens[position] = token;
        position++;

        if (position >= buff_size) {
            buff_size += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, buff_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "glsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}