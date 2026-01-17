#ifndef GLSH_PARSER_H
#define GLSH_PARSER_H

#include "shell.h"

char *glsh_read_line(void);
int glsh_tokenize(const char *line, Token *tokens);
Pipeline *glsh_parse(Token *toks, int ntok);
char **glsh_expand_glob(char **args);

#endif