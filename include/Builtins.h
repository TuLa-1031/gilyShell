#ifndef GLSH_BUILTINS_H
#define GLSH_BUILTINS_H

int glsh_num_builtins(void);
int glsh_builtin_countd(char **args, int is_background);
int glsh_builtin_calculator(char **args, int is_background);
int glsh_builtin_repeat(char **args, int is_background);

extern char *glsh_builtin_names[];
extern int (*glsh_builtin_funcs[])(char **);

#endif