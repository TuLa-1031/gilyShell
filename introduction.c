#include "./include/shell.h"
#include "./include/Builtins.h"
#include "./include/introduction.h"

static char *gilyShellArt[] = {
    "   ____ _ _       ____  _          _ _ ",
    "  / ___(_) |_   _/ ___|| |__   ___| | |",
    " | |  _| | | | | \\___ \\| '_ \\ / _ \\ | |",
    " | |_| | | | |_| |___) | | | |  __/ | |",
    "  \\____|_|_|\\__, |____/|_| |_|\\___|_|_|",
    "            |___/                      "
};

void printAnimatedText() {
    printf("\n");
    int numLines = sizeof(gilyShellArt) / sizeof(gilyShellArt[0]);
    
    for (int i = 0; i < numLines; i++) {
        printf("    %s\n", gilyShellArt[i]);
        
        usleep(50000); 
    }
}

int introduction() {
    system("clear"); 
    
    printf("\n");
    printAnimatedText();
    
    printf("\n");
    printf("    Created by: Le Tung Lam\n");
    printf("    ---------------------------------------\n");
    printf("    Welcome to GilyShell. Type 'help' to start.\n\n");
    
    return 1;
}