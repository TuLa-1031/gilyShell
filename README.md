# Operating Systems – HUST

## Simple UNIX-like Shell

This repository contains my project for the **Operating Systems course at HUST**. The project focuses on building a simple UNIX-like shell from scratch using low-level UNIX system calls.

---

## 📌 Overview

The goal of this project is to understand how an operating system interacts with user-level programs, specifically through:

* Process creation
* Executing external programs
* Managing child processes
* Basic shell behavior (parsing + executing commands)

The shell is implemented in **C**

---

## 📁 Project Structure (Modules)

```
/src
│── main.c              // Entry point of the shell
│── parser.c            // Command parsing (tokenization)
│── launch.c            // Process creation using fork + execvp
│── Builtins.c          // (Reserved) Built-in shell commands
│── introduction.c      // Create introduction artwork

/include
│── shell.h
│── parser.h
│── launch.h
│── Builtins.h
│── introduction.h

Makefile
```

---

## ✨ Features Implemented

### ✔ Process Creation

Uses `fork()` to create a child process for every command.

### ✔ Program Execution

Uses `execvp()` to run external commands such as:

```
ls -l
cat file.txt
gcc main.c
```

### ✔ Command Parsing

Splits user input into tokens before execution.

### ✔ Basic Shell Loop

* Reads input
* Parses command
* Forks & executes
* Waits for child process

---

## 🚧 Features in Progress

* Job control (background processes)
* Signal handling (`SIGINT`, `SIGTSTP`, etc.)
* Built-in commands: `cd`, `exit`, `pwd`
* I/O redirection (`>`, `<`)
* Pipelines (`|`)

---

## 🔧 Build & Run

### Build

```
make
```

### Run

```
./gilyshell
```

---

## 📚 Course Information

* **Course:** Operating Systems (OS)
* **University:** Hanoi University of Science and Technology (HUST)
* **Language:** C
* **Platform:** Linux
