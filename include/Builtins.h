#ifndef BUILTINS_H
#define BUILTINS_H

int help(char **args);
int lsh_num_builtins(void);

extern char *builtin_str[];
extern int (*builtin_func[])(char **args);


#endif