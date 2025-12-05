# GilyShell: A Functional POSIX-Compliant Shell Implementation

**GilyShell** is a custom-built command-line interpreter (CLI) developed in C. Unlike basic shell wrappers, GilyShell is a fully functional environment that implements core Operating System concepts from scratch, including Inter-Process Communication (IPC), Signal Handling, and Process Group Management.

This project demonstrates a deep dive into **Systems Programming**, showcasing the ability to interact directly with the Unix Kernel API without relying on high-level abstractions.

-----

## 🚀 Technical Architecture & Highlights

The project focuses on the interaction between user-space programs and kernel-space services through low-level system calls.

### 1\. Advanced Process Management & Job Control

  * **Background Execution:** Implemented support for background processes using the `&` operator, managing process execution without blocking the shell instance.
  * **Context Switching:** Utilization of `fork()`, `execvp()`, and `waitpid()` to manage the lifecycle of child processes.

### 2\. Inter-Process Communication (IPC) & Pipelines

  * **Piping (`|`):** Engineered a pipeline mechanism using `pipe()` and file descriptor manipulation. This allows the output of one process to serve directly as the input to another (e.g., `ls -l | grep .c`).
  * **Recursive Execution:** Capable of handling multiple piped commands in a single chain.

### 3\. I/O Redirection & File Descriptors

  * **Stream Manipulation:** Implemented standard Unix redirection operators (`>`, `<`, `2>`) using `dup2()` to clone file descriptors, enabling dynamic input/output routing between files and processes.

### 4\. Signal Handling & Safety

  * **Interrupt Management:** robust handling of hardware and software signals.
      * `SIGINT` (Ctrl+C): Safely interrupts the foreground process without crashing the shell.
      * `SIGTSTP` (Ctrl+Z): Manages process suspension.
  * **Zombie Prevention:** Proper cleanup of terminated child processes to prevent resource leaks.

-----

## 📁 Project Structure

Designed with modularity to separate parsing logic from execution strategies.

```
.
├── src/
│   ├── main.c            # Event loop and signal trap initialization
│   ├── parser.c          # Lexical analysis and tokenization logic
│   ├── launch.c          # Process forking and external execution
│   ├── pipes.c           # Logic for handling IPC and piped commands
│   ├── redirect.c        # File descriptor manipulation for I/O
│   ├── Builtins.c        # Internal command implementations (cd, exit, etc.)
│   └── introduction.c    # UI and Shell initialization
├── include/              # Header files defining interfaces for modules
├── Makefile              # Build configuration
└── README.md
```

-----

## ✨ Feature Showcase

### Executing Commands with Pipelines

GilyShell supports chaining commands via pipes:

```bash
gily> ls -l /usr/bin | grep python | wc -l
```

### Input/Output Redirection

Seamlessly redirect streams to files:

```bash
gily> echo "Hello World" > output.txt
gily> cat < output.txt
```

### Background Processes

Run tasks in the background to keep the shell interactive:

```bash
gily> sleep 10 &
[1] 12345
```

-----

## 🛠 Installation & Usage

### Build

Compile the source code using the provided Makefile:

```bash
make
```

### Run

Start the shell instance:

```bash
./gilyshell
```

-----

## 👨‍💻 Author

**Le Tung Lam**