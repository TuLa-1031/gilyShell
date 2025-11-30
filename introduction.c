#include "shell.h"
#include "Builtins.h"
#include "introduction.h"

static char *gilyShellArt[] = {
    "       _ _       _____ _          _ _ ",
    "  __ _(_) |_   / ____| |        | | |",
    " / _` | | | | | | (___ | |__   ___| | |",
    "| (_| | | |y|  \\___ \\| '_ \\ / _ \\ | |",
    " \\__, |_|_| |  ____) | | | |  __/ | |",
    " |___/    |_| |_____/|_| |_|\\___|_|_|",
    "          _/ |                       ",
    "         |__/                        ",
};

void printAnimatedText() {
    printf("\n");
    int numLines = sizeof(gilyShellArt) / sizeof(gilyShellArt[0]);
    for (int i=0;i < numLines;i++){
        printf("%s\n", gilyShellArt[i]);
        usleep(150000);
    }
}

int introduction(){
    system("clear");
    printf("\n\n");
    printAnimatedText();
    printf("\nLe Tung Lam");
    printf("\n_________________________________________________________________________________\n");
    help(NULL);
    return 1;
}
