#include "introduction.h"
#include "shell.h"

static const char *glsh_logo[] = {
    "   ██████╗ ██╗██╗     ██╗   ██╗███████╗██╗  ██╗███████╗██╗     ██╗     ",
    "  ██╔════╝ ██║██║     ╚██╗ ██╔╝██╔════╝██║  ██║██╔════╝██║     ██║     ",
    "  ██║  ███╗██║██║      ╚████╔╝ ███████╗███████║█████╗  ██║     ██║     ",
    "  ██║   ██║██║██║       ╚██╔╝  ╚════██║██╔══██║██╔══╝  ██║     ██║     ",
    "  ╚██████╔╝██║███████╗   ██║   ███████║██║  ██║███████╗███████╗███████╗",
    "   ╚═════╝ ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝"
};

static void print_logo(void) {
  int num_lines = sizeof(glsh_logo) / sizeof(glsh_logo[0]);

  printf("\n");
  for (int i = 0; i < num_lines; i++) {
    printf("    %s\n", glsh_logo[i]);
    usleep(50000);
  }
}

void glsh_introduction(void) {
  system("clear");

  print_logo();

  printf("\n");
  printf("    Created by: Le Tung Lam\n");
  printf("    ---------------------------------------\n");
  printf("    Welcome to GilyShell. Type 'help' to start.\n\n");
}