#ifndef PARSER_H
#define PARSER_H

char *lsh_read_line(void);

char **lsh_split_line(char *line);
char **expand_to_glob_argv(char **args);

#endif