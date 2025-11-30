#include "shell.h"
#include "launch.h"

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid==0) {
        // Child process
        if (execvp(args[0], args)==-1){
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }   else if (pid<0){
        //Error forking
        perror("lsh");
    }   else{
        // Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }   while(!WIFEXITED(status) && !WIFEXITED(status));
    }

    return 1;
}