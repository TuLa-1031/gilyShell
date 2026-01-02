# GilyShell

GilyShell is a custom command-line interpreter developed in C, designed to demonstrate core operating system concepts such as process management, inter-process communication (IPC), and signal handling. It provides a functional shell environment that supports standard POSIX-like syntax and behavior.

This project serves as a practical implementation of system programming concepts, interacting directly with the Unix Kernel API options like `fork`, `exec`, `pipe`, `dup2`, and `ioctl`.

## Features

### 1. Process Management & Job Control
- **Background Execution:** Commands can be run in the background using the `&` operator (e.g., `sleep 10 &`), allowing the shell to remain interactive.
- **Job Control:**
  - `jobs`: List all active background and stopped jobs with their status and Job IDs.
  - `fg`: Bring a background or stopped job to the foreground.
  - `bg`: Resume a stopped job in the background.
  - Support for `Ctrl+Z` (SIGTSTP) to suspend foreground processes and `Ctrl+C` (SIGINT) to interrupt them.

### 2. Pipelines & IPC
- **Piping:** Supports chaining multiple commands using the `|` operator (e.g., `ls | grep .c | wc -l`).
- **Data Flow:** Seamlessly passes standard output from one process to the standard input of the next.

### 3. I/O Redirection
- **Output Redirection:**
  - `>`: Overwrite a file with command output.
  - `>>`: Append command output to a file.
- **Input Redirection:**
  - `<`: Read command input from a file.

### 4. Built-in Commands
GilyShell includes several internal commands that execute within the shell process:

- **Navigation & Environment:**
  - `cd <directory>`: Change the current working directory.
  - `showEnv`: Display all environment variables.
  - `printPath`: Print the current system PATH.
  - `exit`: Terminate the shell session.

- **Utilities:**
  - `history`: Display the list of previously executed commands.
  - `help`: Show the help menu with available commands.
  - `date`: Display the current system date.
  - `time`: Display the current system time.
  - `countd <seconds>`: Start a countdown timer.
  - `calculator`: Launch an external calculator application (demonstrates process isolation and terminal control).

### 5. Signal Handling
- Implements custom signal handlers to manage child processes and shell stability.
- Prevents zombie processes by properly reaping terminated children.
- Ignores interactive signals (like SIGTTOU) when necessary to maintain terminal control.

## Installation and Usage

### Prerequisites
- A Unix-like operating system (Linux, macOS).
- GCC compiler.
- Make build system.

### Compiling
To build the shell, run the following command in the project root:

```sh
make
```

 This will compile all source files and link them into an executable named `gilyshell`.

### Running
Start the shell by running:

```sh
./gilyshell
```

To remove build artifacts:

```sh
make clean
```

## Architecture Overview

The shell operates through a standard Read-Eval-Print Loop (REPL):

1.  **Initialization:** Sets up job list, history, and signal traps.
2.  **Read:** Captures user input from the command line.
3.  **Parse:** Tokenizes the input and constructs a `Pipeline` structure containing one or more `Command` objects. Identifies operators like `&`, `|`, `>`, `>>`, `<`.
4.  **Execute:**
    - Checks for built-in commands.
    - If external, `fork()` is called to create child processes.
    - `execvp()` replaces the child process image with the requested command.
    - Manages file descriptors for pipes and redirection using `dup2()`.
    - Handles foreground/background execution logic using `waitpid()` and process groups.
5.  **Loop:** Returns to the prompt for the next command.

## Project Structure

- `src/`: Source files (`main.c`, `parser.c`, `execute.c`, `Builtins.c`, `jobs.c`, `utils.c`, `introduction.c`).
- `include/`: Header files defining data structures and function prototypes.
- `Makefile`: Build instructions.

## Author

Le Tung Lam