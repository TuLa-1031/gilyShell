#include "shell.h"
#include "Builtins.h"
#include "introduction.h"

#define COLOR_CYAN  "\033[1;36m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"

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
    
    printf("%s", COLOR_CYAN); 
    
    for (int i = 0; i < numLines; i++) {
        printf("    %s\n", gilyShellArt[i]);
        
        usleep(50000); 
    }
    
    printf("%s", COLOR_RESET);
}

int introduction() {
    system("clear"); 
    
    printf("\n");
    printAnimatedText();
    
    printf("\n");
    printf("    %sCreated by:%s Le Tung Lam\n", COLOR_YELLOW, COLOR_RESET);
    printf("    %s---------------------------------------%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    Welcome to GilyShell. Type 'help' to start.\n\n");
    
    return 1;
}