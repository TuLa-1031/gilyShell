#include "introduction.h"
#include "Builtins.h"
#include "shell.h"

static const char *gilyShellArt[] = {
    "   ██████╗ ██╗██╗     ██╗   ██╗███████╗██╗  ██╗███████╗██╗     ██╗     ",
    "  ██╔════╝ ██║██║     ╚██╗ ██╔╝██╔════╝██║  ██║██╔════╝██║     ██║     ",
    "  ██║  ███╗██║██║      ╚████╔╝ ███████╗███████║█████╗  ██║     ██║     ",
    "  ██║   ██║██║██║       ╚██╔╝  ╚════██║██╔══██║██╔══╝  ██║     ██║     ",
    "  ╚██████╔╝██║███████╗   ██║   ███████║██║  ██║███████╗███████╗███████╗",
    "   ╚═════╝ ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝"};

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