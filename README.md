# smallsh - A Custom Shell in C

## Overview
`smallsh` is a simple shell implemented in C that supports:
- Command execution
- Input/output redirection (`<`, `>`)
- Background execution (`&`)
- Built-in commands: `exit`, `cd`, `status`
- Signal handling (`SIGINT` for Ctrl+C, `SIGTSTP` for Ctrl+Z)

## Installation & Compilation
Clone the repository and compile the program using GCC:
```sh
gcc -o smallsh smallsh.c
```
OR use the Makefile:
```sh
make
```
to clean:
```sh
make clean
```

## Usage:
Run the shell:
```sh
./smallsh
```

## Example Commands:
```sh
: ls
: cd ..
: echo "Hello" > file.txt
: cat < file.txt
: sleep 10 &
: exit
```

## Features:
- Foreground/Background execution: Run commands in the background using &.
- Input/Output redirection: Use < and > for file I/O.
- Signal handling:
    - Ctrl+C (SIGINT): Kills only foreground processes.
    - Ctrl+Z (SIGTSTP): Toggles foreground-only mode.