#ifndef EXECUTE_H
#define EXECUTE_H

#include "shell.h"

int lsh_execute(char **args);
int execute_pipeline(Pipeline *pl);

#endif